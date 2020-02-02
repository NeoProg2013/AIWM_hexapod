//  ***************************************************************************
/// @file    system_status.c
/// @author  NeoProg
//  ***************************************************************************
#include "system_monitor.h"
#include "adc.h"

#define BATTERY_VOLTAGE_THRESHOLD       (900)       // 9.00V

typedef enum {
    STATE_NO_INIT,
    STATE_START_CONVERSION,
    STATE_WAIT,
    STATE_CALCULATION,
    STATE_CHECK
} monitor_state_t;

static monitor_state_t monitor_state = STATE_NO_INIT;


uint8_t  sysmon_system_status = 0;
uint8_t  sysmon_module_status = 0;
uint16_t sysmon_battery_cell_voltage[3] = {0, 0, 0};


//  ***************************************************************************
/// @brief  System monitor initialization
/// @param  none
/// @return none
//  ***************************************************************************
void sysmon_init(void) {

    sysmon_system_status = 0;
    sysmon_module_status = 0;

    // ADC initialization
    adc_init();
    monitor_state = STATE_START_CONVERSION;
}

//  ***************************************************************************
/// @brief  System monitor process
/// @note   Call from Main loop
/// @return none
//  ***************************************************************************
void sysmon_process(void) {

    if (sysmon_is_module_disable(SYSMON_MODULE_SYSTEM_MONITOR) == true) {
        sysmon_battery_cell_voltage[0] = 0;
        sysmon_battery_cell_voltage[1] = 0;
        sysmon_battery_cell_voltage[2] = 0;
        return;
    }

    uint32_t battery_voltage = 0;
    switch (monitor_state) {

        case STATE_START_CONVERSION:
            adc_start_conversion();
            monitor_state = STATE_WAIT;
            break;

        case STATE_WAIT:
            if (adc_is_conversion_complete() == true) {
                monitor_state = STATE_CALCULATION;
            }
            break;

        case STATE_CALCULATION:
            sysmon_battery_cell_voltage[0] = adc_get_conversion_result(0);
            sysmon_battery_cell_voltage[1] = adc_get_conversion_result(1);
            sysmon_battery_cell_voltage[2] = adc_get_conversion_result(2);
            
            /*battery_voltage += sysmon_battery_cell_voltage[0];
            battery_voltage += sysmon_battery_cell_voltage[1];
            battery_voltage += sysmon_battery_cell_voltage[2];*/
            monitor_state = STATE_CHECK;
            break;

        case STATE_CHECK:
            /*if (battery_voltage < BATTERY_VOLTAGE_THRESHOLD) {
                sysmon_set_error(SYSMON_VOLTAGE_ERROR);
            }*/
            monitor_state = STATE_START_CONVERSION;
            break;

        case STATE_NO_INIT:
        default:
            sysmon_set_error(SYSMON_FATAL_ERROR);
            sysmon_disable_module(SYSMON_MODULE_SYSTEM_MONITOR);
            break;
    }
}

//  ***************************************************************************
/// @brief  Set error to system status
/// @param  error: error mask. @ref sysmon_error_t
/// @return none
//  ***************************************************************************
void sysmon_set_error(sysmon_error_t error) {
    sysmon_system_status |= error;

}

//  ***************************************************************************
/// @brief  Clear error from system status
/// @param  error: error mask. @ref sysmon_error_t
/// @return none
//  ***************************************************************************
void sysmon_clear_error(sysmon_error_t error) {
    sysmon_system_status &= ~error;
}

//  ***************************************************************************
/// @brief  Disable module
/// @param  module: module mask. @ref sysmon_module_t
/// @return none
//  ***************************************************************************
void sysmon_disable_module(sysmon_module_t module) {
    sysmon_module_status |= module;
}

//  ***************************************************************************
/// @brief  Enable module
/// @param  module: module mask. @ref sysmon_module_t
/// @return none
//  ***************************************************************************
void sysmon_enable_module(sysmon_module_t module) {
    sysmon_module_status &= ~module;
}

//  ***************************************************************************
/// @brief  Check errors in system status
/// @param  error: error mask. @ref sysmon_error_t
/// @return true - error is set, false - no
//  ***************************************************************************
bool sysmon_is_error_set(sysmon_error_t error) {
    return sysmon_system_status & error;
}

//  ***************************************************************************
/// @brief  Check module disable bits
/// @param  module: module mask. @ref sysmon_module_t
/// @return true - bit is set, false - no
//  ***************************************************************************
bool sysmon_is_module_disable(sysmon_module_t module) {
    return sysmon_module_status & module;
}

