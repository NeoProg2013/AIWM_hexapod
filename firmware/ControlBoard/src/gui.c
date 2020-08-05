//  ***************************************************************************
/// @file    gui.c
/// @author  NeoProg
//  ***************************************************************************
#include "gui.h"
#include <stdint.h>
#include <stdbool.h>
#include "oled_gl.h"
#include "system_monitor.h"
#include "systimer.h"
#include "version.h"

#define DISPLAY_UPDATE_PERIOD                   (500)


#define BATTERY_BITMAP_WIDTH                    (13)
#define BATTERY_BITMAP_HEIGHT                   (8)
static const uint8_t battery_bitmap[] = {
    0xFE, 0x82, 0x93, 0xBB, 0x92, 0x82, 0x82, 0x82, 0x92, 0x93, 
    0x93, 0x82, 0xFE
};

#define SYSTEM_BITMAP_WIDTH                     (13)
#define SYSTEM_BITMAP_HEIGHT                    (8)
static const uint8_t system_bitmap[] = {
    0x7E, 0xC3, 0x42, 0xC3, 0x42, 0xC3, 0x42, 0xC3, 0x42, 0xC3, 
    0x42, 0xC3, 0x7E
};

#define MODULE_BITMAP_WIDTH                     (13)
#define MODULE_BITMAP_HEIGHT                    (8)
static const uint8_t module_bitmap[] = {
    0xE7, 0xE7, 0xE7, 0x00, 0x00, 0xE7, 0xE7, 0xE7, 0x00, 0x00, 
    0xE7, 0xE7, 0xE7
};

typedef enum {
    STATE_NOINIT,
    STATE_UPDATE_BATTERY_CHARGE,
    STATE_UPDATE_SYSTEM_STATUS,
    STATE_UPDATE_MODULE_STATUS,
    STATE_UPDATE_SYSTEM_MODE,
    STATE_UPDATE_DISPLAY
} state_t;

static state_t module_state = STATE_NOINIT;


//  ***************************************************************************
/// @brief  GUI initialization
/// @param  none
/// @return none
//  ***************************************************************************
void gui_init(void) {
    
    oled_gl_init();
  
    // Draw battery voltage (static)
    oled_gl_draw_bitmap(0, 0, BATTERY_BITMAP_WIDTH, BATTERY_BITMAP_HEIGHT, battery_bitmap);
    oled_gl_draw_dec_number(0, 18, 99);
    oled_gl_draw_string(0, 35, "%");
    
    // Draw system status (static)
    oled_gl_draw_bitmap(2, 0, SYSTEM_BITMAP_WIDTH, SYSTEM_BITMAP_HEIGHT, system_bitmap);
    oled_gl_draw_hex16(2, 18, 0x0000);
    
    // Draw module status (static)
    oled_gl_draw_bitmap(4, 0, MODULE_BITMAP_WIDTH, MODULE_BITMAP_HEIGHT, module_bitmap);
    oled_gl_draw_hex16(4, 18, 0x0000);

    // Draw horizontal separator (static)
    oled_gl_draw_horizontal_line(5, 0, 7, 128);
    
    // Draw error status (dynamic)
    char fw_version[24] = FIRMWARE_VERSION;
    fw_version[19] = '\0';
    oled_gl_draw_string(7, 4, fw_version);
    
    // Draw vertical separator (static)
    oled_gl_draw_string(0, 58, "|");
    oled_gl_draw_string(1, 58, "|");
    oled_gl_draw_string(2, 58, "|");
    oled_gl_draw_string(3, 58, "|");
    oled_gl_draw_string(4, 58, "|");
    
    // Draw system mode (dynamic)
    oled_gl_draw_string(0, 67, "SYSTEM");
    oled_gl_draw_string(2, 67, "INIT");
    oled_gl_draw_string(4, 67, "MODE");
    
    // Draw system active indicator (dynamic)
    oled_gl_draw_rect(0, 120, 0, 8, 8);
    
    oled_gl_sync_display_update();
    module_state = STATE_UPDATE_BATTERY_CHARGE;
}

//  ***************************************************************************
/// @brief  GUI process
/// @param  none
/// @return none
//  ***************************************************************************
void gui_process(void) {
    
    if (sysmon_is_module_disable(SYSMON_MODULE_GUI) == true) return;
    
    
    static uint32_t prev_update_time = 0;
    static bool is_system_active_indicator_visible = false;
    
    switch (module_state) {
        
        case STATE_UPDATE_BATTERY_CHARGE:
            oled_gl_draw_dec_number(0, 18, sysmon_battery_charge);
            module_state = STATE_UPDATE_SYSTEM_STATUS;
            break;
        
        case STATE_UPDATE_SYSTEM_STATUS:
            oled_gl_draw_hex16(2, 18, 0x0000);
            module_state = STATE_UPDATE_MODULE_STATUS;
            break;
            
        case STATE_UPDATE_MODULE_STATUS:
            oled_gl_draw_hex16(4, 18, 0x0000);
            module_state = STATE_UPDATE_SYSTEM_MODE;
            break;
            
        case STATE_UPDATE_SYSTEM_MODE:
            oled_gl_clear_row_fragment(0, 67, 0, 50, 8);
            oled_gl_clear_row_fragment(2, 67, 0, 67, 8);
            oled_gl_clear_row_fragment(4, 67, 0, 67, 8);
            if (sysmon_is_error_set(SYSMON_FATAL_ERROR) == true) {
                
                if (is_system_active_indicator_visible == true) {
                    oled_gl_draw_string(0, 67, "SYSTEM");
                    oled_gl_draw_string(2, 67, "EMERGENCY");
                    oled_gl_draw_string(4, 67, "MODE");
                }
            }
            else if (sysmon_is_error_set(SYSMON_VOLTAGE_ERROR) == true) {
                
                if (is_system_active_indicator_visible == true) {
                    oled_gl_draw_string(0, 67, "BATTERY");
                    oled_gl_draw_string(2, 67, "LOW");
                    oled_gl_draw_string(4, 67, "VOLTAGE");
                }
            }
            else {
                oled_gl_draw_string(0, 67, "SYSTEM");
                oled_gl_draw_string(2, 67, "STANDART");
                oled_gl_draw_string(4, 67, "MODE");
            }
            module_state = STATE_UPDATE_DISPLAY;
            break;
            
        case STATE_UPDATE_DISPLAY:
            if (get_time_ms() - prev_update_time >= DISPLAY_UPDATE_PERIOD) {
                
                // Blink system active indicator
                if (is_system_active_indicator_visible == true) {
                    oled_gl_clear_row_fragment(0, 120, 0, 8, 8);
                }
                else {
                    oled_gl_draw_rect(0, 120, 0, 8, 8);
                }
                is_system_active_indicator_visible = !is_system_active_indicator_visible;
                
                // Start display update
                oled_gl_start_async_display_update();
                prev_update_time = get_time_ms();
                
                module_state = STATE_UPDATE_BATTERY_CHARGE;
            }
            break;
        
        case STATE_NOINIT:
        default:
            sysmon_set_error(SYSMON_FATAL_ERROR);
            sysmon_disable_module(SYSMON_MODULE_GUI);
            break;
    }
    
    oled_gl_process();
}
