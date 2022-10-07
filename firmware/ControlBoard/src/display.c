/// ***************************************************************************
/// @file    display.c
/// @author  NeoProg
/// ***************************************************************************
#include "project-base.h"
#include "display.h"
#include "oled-gl.h"
#include "system-monitor.h"
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
    STATE_WAIT,
    STATE_UPDATE_BATTERY_CHARGE,
    STATE_UPDATE_SYSTEM_STATUS,
    STATE_UPDATE_MODULE_STATUS,
    STATE_UPDATE_SYSTEM_MODE,
    STATE_UPDATE_DISPLAY_START,
    STATE_UPDATE_DISPLAY_PROCESS,
} state_t;

static state_t module_state = STATE_NOINIT;


/// ***************************************************************************
/// @brief  Display initialization
/// @param  none
/// @return none
/// ***************************************************************************
void display_init(void) {
    if (!oled_gl_init()) {
        sysmon_set_error(SYSMON_I2C_ERROR);
        sysmon_disable_module(SYSMON_MODULE_DISPLAY);
        return;
    }
  
    // Draw battery voltage (static)
    if (!oled_gl_draw_bitmap(0, 0, BATTERY_BITMAP_WIDTH, BATTERY_BITMAP_HEIGHT, battery_bitmap)) {
        sysmon_set_error(SYSMON_INTERNAL_ERROR);
        sysmon_disable_module(SYSMON_MODULE_DISPLAY);
        return;
    }
    oled_gl_draw_dec_number(0, 18, 99);
    oled_gl_draw_string(0, 35, "%");
    
    // Draw system status (dynamic)
    if (!oled_gl_draw_bitmap(2, 0, SYSTEM_BITMAP_WIDTH, SYSTEM_BITMAP_HEIGHT, system_bitmap)) {
        sysmon_set_error(SYSMON_FATAL_ERROR);
        sysmon_disable_module(SYSMON_MODULE_DISPLAY);
        return;
    }
    oled_gl_draw_hex(2, 18, 0x0000, 4);
    
    // Draw module status (dynamic)
    if (!oled_gl_draw_bitmap(4, 0, MODULE_BITMAP_WIDTH, MODULE_BITMAP_HEIGHT, module_bitmap)) {
        sysmon_set_error(SYSMON_INTERNAL_ERROR);
        sysmon_disable_module(SYSMON_MODULE_DISPLAY);
        return;
    }
    oled_gl_draw_hex(4, 18, 0x0000, 4);

    // Draw horizontal separator (static)
    oled_gl_draw_rect(5, 0, 7, 128, 1);
    
    // Draw firmware version (static) 
    // Example: 1.0.200825 100773
    char fw_version[] = FIRMWARE_VERSION;
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
    
    if (!oled_gl_sync_update()) {
        sysmon_set_error(SYSMON_I2C_ERROR);
        sysmon_disable_module(SYSMON_MODULE_DISPLAY);
        return;
    }
    module_state = STATE_WAIT;
}

/// ***************************************************************************
/// @brief  Display process
/// @param  none
/// @return none
/// ***************************************************************************
void display_process(void) {
    if (sysmon_is_module_disable(SYSMON_MODULE_DISPLAY)) return;
    
    
    static uint32_t prev_update_time = 0;
    static bool is_system_active_indicator_visible = false;
    
    switch (module_state) {
        case STATE_WAIT:
            if (get_time_ms() - prev_update_time >= DISPLAY_UPDATE_PERIOD) {
                module_state = STATE_UPDATE_BATTERY_CHARGE;
            }  
            break;
        
        case STATE_UPDATE_BATTERY_CHARGE:
            oled_gl_clear_fragment(0, 18, 17);
            oled_gl_draw_dec_number(0, 18, sysmon_battery_charge);
            module_state = STATE_UPDATE_SYSTEM_STATUS;
            break;
        
        case STATE_UPDATE_SYSTEM_STATUS:
            oled_gl_draw_hex(2, 18, sysmon_system_status, 4);
            module_state = STATE_UPDATE_MODULE_STATUS;
            break;
            
        case STATE_UPDATE_MODULE_STATUS:
            oled_gl_draw_hex(4, 18, sysmon_module_status, 4);
            module_state = STATE_UPDATE_SYSTEM_MODE;
            break;
            
        case STATE_UPDATE_SYSTEM_MODE:
            oled_gl_clear_fragment(0, 67, 67);
            oled_gl_clear_fragment(2, 67, 67);
            oled_gl_clear_fragment(4, 67, 67);
            if (sysmon_is_error_set(SYSMON_FATAL_ERROR) == true) {
                if (is_system_active_indicator_visible == true) {
                    oled_gl_draw_string(0, 67, "SYSTEM");
                    oled_gl_draw_string(2, 67, "EMERGENCY");
                    oled_gl_draw_string(4, 67, "MODE");
                }
            } else if (sysmon_is_error_set(SYSMON_VOLTAGE_ERROR) == true) {
                if (is_system_active_indicator_visible == true) {
                    oled_gl_draw_string(0, 67, "BATTERY");
                    oled_gl_draw_string(2, 67, "LOW");
                    oled_gl_draw_string(4, 67, "VOLTAGE");
                }
            } else if (sysmon_is_error_set(SYSMON_CALIBRATION) == true) {
                if (is_system_active_indicator_visible == true) {
                    oled_gl_draw_string(0, 67, "SYSTEM");
                    oled_gl_draw_string(2, 67, "CALIBRATE");
                    oled_gl_draw_string(4, 67, "MODE");
                }
            } else {
                oled_gl_draw_string(0, 67, "SYSTEM");
                oled_gl_draw_string(2, 67, "STANDART");
                oled_gl_draw_string(4, 67, "MODE");
            }
            
            if (is_system_active_indicator_visible) {
                oled_gl_draw_rect(0, 120, 0, 8, 8);
            } else {
                oled_gl_clear_fragment(0, 120, 8);
            }
            
            is_system_active_indicator_visible = !is_system_active_indicator_visible;
            module_state = STATE_UPDATE_DISPLAY_START;
            break;
            
        case STATE_UPDATE_DISPLAY_START:
            oled_gl_async_update();
            module_state = STATE_UPDATE_DISPLAY_PROCESS;           
            break;
            
        case STATE_UPDATE_DISPLAY_PROCESS:
            if (oled_gl_is_async_update_completed()) {
                prev_update_time = get_time_ms();
                module_state = STATE_WAIT;
            }
            break;
        
        case STATE_NOINIT:
        default:
            sysmon_set_error(SYSMON_INTERNAL_ERROR);
            sysmon_disable_module(SYSMON_MODULE_DISPLAY);
            break;
    }
    
    if (!oled_gl_async_process()) {
        sysmon_set_error(SYSMON_I2C_ERROR);
        sysmon_disable_module(SYSMON_MODULE_DISPLAY);
        return;
    }
}
