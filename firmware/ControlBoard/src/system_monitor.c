//  ***************************************************************************
/// @file    system_status.c
/// @author  NeoProg
//  ***************************************************************************
#include "system_monitor.h"
#include "adc.h"
#include "systimer.h"

#define PAUSE_BETWEEN_CONVERSIONS       (5)
#define ACCUMULATE_SAMPLES_COUNT        (100)

#define ADC_CHANNELS_COUNT              (1)
#define ADC_BATTERY_VOLTAGE_CH          (0)


typedef enum {
    STATE_NO_INIT,
    STATE_START_CONVERSION,
    STATE_WAIT,
    STATE_ACCUMULATE,
    STATE_CALCULATION,
    STATE_PAUSE
} monitor_state_t;


static monitor_state_t monitor_state = STATE_NO_INIT;
static uint32_t acc_adc_bins[ADC_CHANNELS_COUNT] = {0};
static int16_t  battery_voltage_offset = 0;

uint8_t  sysmon_system_status = 0;
uint8_t  sysmon_module_status = 0;
uint16_t sysmon_battery_voltage = 0;
uint8_t  sysmon_battery_charge = 0;


static void calculate_battery_voltage(void);


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
        sysmon_battery_charge = 0;
        sysmon_battery_voltage = 0;
        return;
    }

    
    static uint64_t pause_start_time = 0;
    static uint32_t accumulate_counter = 0;
    switch (monitor_state) {

        case STATE_START_CONVERSION:
            adc_start_conversion();
            monitor_state = STATE_WAIT;
            break;

        case STATE_WAIT:
            if (adc_is_conversion_complete() == true) {
                monitor_state = STATE_ACCUMULATE;
            }
            break;
            
        case STATE_ACCUMULATE:
            acc_adc_bins[ADC_BATTERY_VOLTAGE_CH] += adc_get_conversion_result(ADC_BATTERY_VOLTAGE_CH);
            if (++accumulate_counter >= ACCUMULATE_SAMPLES_COUNT) {
                accumulate_counter = 0;
                monitor_state = STATE_CALCULATION;
            }
            else {
                pause_start_time = get_time_ms();
                monitor_state = STATE_PAUSE;
            }
            break;

        case STATE_CALCULATION:
            calculate_battery_voltage();
            acc_adc_bins[ADC_BATTERY_VOLTAGE_CH] = 0;
            if (sysmon_battery_charge == 0) {
                sysmon_set_error(SYSMON_VOLTAGE_ERROR);
            }
            pause_start_time = get_time_ms();
            monitor_state = STATE_PAUSE;
            break;
            
        case STATE_PAUSE:
            if (get_time_ms() - pause_start_time > PAUSE_BETWEEN_CONVERSIONS) {
                monitor_state = STATE_START_CONVERSION;
            }
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
/// @param  error: error mask
/// @return none
//  ***************************************************************************
void sysmon_set_error(uint32_t error) {
    sysmon_system_status |= error;
}

//  ***************************************************************************
/// @brief  Clear error from system status
/// @param  error: error mask
/// @return none
//  ***************************************************************************
void sysmon_clear_error(uint32_t error) {
    sysmon_system_status &= ~error;
}

//  ***************************************************************************
/// @brief  Disable module
/// @param  module: module mask
/// @return none
//  ***************************************************************************
void sysmon_disable_module(uint32_t module) {
    sysmon_module_status |= module;
}

//  ***************************************************************************
/// @brief  Enable module
/// @param  module: module mask
/// @return none
//  ***************************************************************************
void sysmon_enable_module(uint32_t module) {
    sysmon_module_status &= ~module;
}

//  ***************************************************************************
/// @brief  Check errors in system status
/// @param  error: error mask
/// @return true - error is set, false - no
//  ***************************************************************************
bool sysmon_is_error_set(uint32_t error) {
    return sysmon_system_status & error;
}

//  ***************************************************************************
/// @brief  Check module disable bits
/// @param  module: module mask
/// @return true - bit is set, false - no
//  ***************************************************************************
bool sysmon_is_module_disable(uint32_t module) {
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
    
    // Battery voltage (max voltage 12.6V)
    uint32_t avr_adc_bins = acc_adc_bins[ADC_BATTERY_VOLTAGE_CH] / ACCUMULATE_SAMPLES_COUNT;
    float input_voltage = avr_adc_bins * bins_to_voltage_factor;
    int32_t battery_voltage = (uint32_t)(input_voltage * voltage_div_factor);

    // Offset battery voltage
    sysmon_battery_voltage = battery_voltage;
    sysmon_battery_voltage += battery_voltage_offset;
    
    // Calculate battery charge persents
    float battery_charge = (sysmon_battery_voltage - 9000.0f) / (12600.0f - 9000.0f) * 100.0f;
    if (battery_charge < 0) {
        battery_charge = 0;
    }
    if (battery_charge > 99) {
        battery_charge = 99;
    }
    sysmon_battery_charge = (uint8_t)battery_charge;
}
