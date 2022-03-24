/// ***************************************************************************
/// @file    servo-driver.c
/// @author  NeoProg
/// ***************************************************************************
#include "project-base.h"
#include "sensors-core.h"
#include "pca9555.h"
#include "mpu6050.h"
#include "system-monitor.h"
#include "systimer.h"



/// ***************************************************************************
/// @brief  Sensors core initialization
/// ***************************************************************************
void sensors_core_init(void) {
    if (!mpu6050_init() || !mpu6050_set_state(true)) {
        sysmon_set_error(SYSMON_I2C_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MPU6050);
    }
}
bool sensors_core_calibration_process(void) {
    static uint64_t start_calibration_time = 0;
    if (start_calibration_time == 0) {
        start_calibration_time = get_time_ms();
    }
    
    if (get_time_ms() - start_calibration_time < 25000) {
        if (mpu6050_is_data_ready()) {
            float dummy[2] = {0};
            if (!mpu6050_read_data(dummy)) {
                sysmon_set_error(SYSMON_I2C_ERROR);
                sysmon_disable_module(SYSMON_MODULE_MPU6050);
                return false;
            }
        }
        return true;
    }
    return false;
}
void sensors_core_process(void) {
    /* if (pca9555_is_input_changed()) {
            asm("nop");
        }*/
}
