//  ***************************************************************************
/// @file    system_status.c
/// @author  NeoProg
//  ***************************************************************************
#include "system_monitor.h"
#include "adc.h"

#define BATTERY_VOLTAGE_THRESHOLD       (900)       // 9.00V
#define CELL_VOLTAGE_THRESHOLD          (280)       // 2.80V


typedef enum {
    STATE_NO_INIT,
    STATE_START_CONVERSION,
    STATE_WAIT,
    STATE_CALCULATION
} monitor_state_t;


static monitor_state_t monitor_state = STATE_NO_INIT;

static void calculate_battery_voltage(void);


uint8_t  sysmon_system_status = 0;
uint8_t  sysmon_module_status = 0;
uint16_t sysmon_battery_cell_voltage[3] = {0, 0, 0};
uint16_t sysmon_battery_voltage = 0;


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
        sysmon_battery_voltage = 0;
        return;
    }

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
            calculate_battery_voltage();
            if (sysmon_battery_voltage < BATTERY_VOLTAGE_THRESHOLD) {
                sysmon_set_error(SYSMON_VOLTAGE_ERROR);
            }
            if (sysmon_battery_cell_voltage[0] < CELL_VOLTAGE_THRESHOLD) {
                sysmon_set_error(SYSMON_VOLTAGE_ERROR);
            }
            if (sysmon_battery_cell_voltage[1] < CELL_VOLTAGE_THRESHOLD) {
                sysmon_set_error(SYSMON_VOLTAGE_ERROR);
            }
            if (sysmon_battery_cell_voltage[2] < CELL_VOLTAGE_THRESHOLD) {
                sysmon_set_error(SYSMON_VOLTAGE_ERROR);
            }
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





//  ***************************************************************************
/// @brief  Calculate battery voltage 
/// @param  none
/// @return none
//  ***************************************************************************
static void calculate_battery_voltage(void) {
    
    // Revert voltage divisor factor (voltage_div_factor = 1 / real_factor)
    // Voltage divisor: VIN-[10k]-OUT-[3k3]-GND
    // * 1000 - convert V to mV
    const float voltage_div_factor = ((10000.0f + 3300.0f) / 3300.0f) * 1000.0f;
    const float bins_to_voltage_factor = 3.3f / 4096.0f;
    
    uint32_t adc_bins = 0;
    float input_voltage = 0;
    
    // Battery cell #1 (max voltage 4.2V)
    adc_bins = adc_get_conversion_result(0);
    input_voltage = adc_bins * bins_to_voltage_factor;
    sysmon_battery_cell_voltage[0] = (uint32_t)(input_voltage * voltage_div_factor);
    
    // Battery cell #2 (max voltage 8.4V)
    adc_bins = adc_get_conversion_result(1);
    input_voltage = adc_bins * bins_to_voltage_factor;
    sysmon_battery_cell_voltage[1] = (uint32_t)(input_voltage * voltage_div_factor);
    
    // Battery cell #3 (max voltage 12.6V)
    adc_bins = adc_get_conversion_result(2);
    input_voltage = adc_bins * bins_to_voltage_factor;
    sysmon_battery_cell_voltage[2] = (uint32_t)(input_voltage * voltage_div_factor);
    
    sysmon_battery_voltage = sysmon_battery_cell_voltage[2];
    sysmon_battery_cell_voltage[2] -= sysmon_battery_cell_voltage[1];
    sysmon_battery_cell_voltage[1] -= sysmon_battery_cell_voltage[0];
}
