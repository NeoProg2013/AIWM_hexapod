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


typedef enum {
    STATE_NOINIT,
    STATE_UPDATE_BATTERY_VOLTAGE,
    STATE_UPDATE_BATTERY_CHARGE,
    STATE_UPDATE_SYSTEM_STATUS,
    STATE_UPDATE_MODULE_STATUS,
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
  
    // Draw battery voltage
    oled_gl_draw_bitmap(0, 0, BATTERY_BITMAP_WIDTH, BATTERY_BITMAP_HEIGHT, battery_bitmap);
    oled_gl_draw_float_number(0, 20, 12.6);
    oled_gl_draw_string(0, 45, "V");
    
    // Draw battery charge voltage
    oled_gl_draw_bitmap(2, 0, BATTERY_BITMAP_WIDTH, BATTERY_BITMAP_HEIGHT, battery_bitmap);
    oled_gl_draw_float_number(2, 20, 100);
    oled_gl_draw_string(2, 45, "%");

    // Draw horizontal separator
    oled_gl_draw_horizontal_line(5, 0, 7, 128);
    
    // Draw error status
    oled_gl_draw_hex16(7, 0, 0x0000);
    
    // Draw FW version number
    oled_gl_draw_string(7, 0, FIRMWARE_VERSION);
    
    // Draw vertical separator
    oled_gl_draw_string(0, 56, "|");
    oled_gl_draw_string(1, 56, "|");
    oled_gl_draw_string(2, 56, "|");
    oled_gl_draw_string(3, 56, "|");
    oled_gl_draw_string(4, 56, "|");
    
    // Draw system mode
    oled_gl_draw_string(0, 67, "SYSTEM");
    oled_gl_draw_string(2, 67, "NORMAL");
    oled_gl_draw_string(4, 67, "MODE");
    
    oled_gl_sync_display_update();
    
    module_state = STATE_UPDATE_BATTERY_VOLTAGE;
}

//  ***************************************************************************
/// @brief  GUI process
/// @param  none
/// @return none
//  ***************************************************************************
void gui_process(void) {
    
    /*if (callback_is_gui_error_set() == true) return;
    
    
    static uint32_t prev_update_time = 0;
    
    switch (module_state) {
        
        case STATE_UPDATE_BATTERY_VOLTAGE:
            oled_gl_draw_float_number(0, 20, battery_voltage / 10.0);
            module_state = STATE_UPDATE_PERIPHERY_VOLTAGE;
            break;
        
        case STATE_UPDATE_PERIPHERY_VOLTAGE:
            oled_gl_draw_float_number(2, 20, sensors_voltage / 10.0);
            module_state = STATE_UPDATE_WIRELESS_VOLTAGE;
            break;
            
        case STATE_UPDATE_WIRELESS_VOLTAGE:
            oled_gl_draw_float_number(4, 20, wireless_voltage / 10.0);
            module_state = STATE_UPDATE_ERROR_STATUS;
            break;
            
        case STATE_UPDATE_ERROR_STATUS:
            oled_gl_draw_hex_number(7, 0, error_status);
            module_state = STATE_UPDATE_SYSTEM_MODE;
            break;
            
        case STATE_UPDATE_SYSTEM_MODE:
            if (callback_is_emergency_mode_active() == true) {
                oled_gl_clear_row_fragment(2, 67, 0, 67, 8);
                oled_gl_draw_string(2, 67, "EMERGENCY");
            }
            else if (callback_is_voltage_error_set() == true) {
                oled_gl_draw_string(0, 67, "BATTERY");
                oled_gl_draw_string(2, 67, "LOW");
                oled_gl_draw_string(4, 67, "VOLTAGE");
            }
            else {
                oled_gl_clear_row_fragment(2, 67, 0, 67, 8);
                oled_gl_draw_string(2, 67, "STANDART");
            }
            module_state = STATE_UPDATE_DISPLAY;
            break;
            
        case STATE_UPDATE_DISPLAY:
            if (get_time_ms() - prev_update_time >= DISPLAY_UPDATE_PERIOD) {
                
                static bool is_rect_visible = false;
                if (is_rect_visible == true) {
                    oled_gl_clear_row_fragment(0, 120, 0, 8, 8);
                }
                else {
                    oled_gl_draw_rect(0, 120, 0, 8, 8);
                }
                is_rect_visible = !is_rect_visible;
                
                oled_gl_start_async_display_update();
                prev_update_time = get_time_ms();
                
                module_state = STATE_UPDATE_BATTERY_VOLTAGE;
            }
            break;
        
        case STATE_NOINIT:
        default:
            callback_set_internal_error(ERROR_MODULE_GUI);
            break;
    }
    */
    
    oled_gl_process();
}
