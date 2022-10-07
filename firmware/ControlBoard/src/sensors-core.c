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

uint16_t sensors_inputs = 0;
static float mpu6050_flt_data[2] = {0};


/// ***************************************************************************
/// @brief  Sensors core initialization
/// ***************************************************************************
void sensors_core_init(void) {
    if (!mpu6050_init() || !mpu6050_set_state(true)) {
        sysmon_set_error(SYSMON_I2C_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MPU6050);
    }
    if (!pca9555_init()) {
        sysmon_set_error(SYSMON_I2C_ERROR);
        sysmon_disable_module(SYSMON_MODULE_PCA9555);
    }
}
bool sensors_core_calibration_process(void) {
    if (sysmon_is_module_disable(SYSMON_MODULE_MPU6050)) return false;
        
    static uint64_t start_calibration_time = 0;
    if (start_calibration_time == 0) {
        start_calibration_time = get_time_ms();
    }
    
    static uint32_t errors_count = 0;
    if (get_time_ms() - start_calibration_time < 25000) {
        if (mpu6050_is_data_ready()) {
            if (!mpu6050_read_data(mpu6050_flt_data)) {
               if (++errors_count > 10) {
                   sysmon_set_error(SYSMON_I2C_ERROR);
                   sysmon_disable_module(SYSMON_MODULE_MPU6050);
                   return false;
               }
            } else {
                errors_count = 0;
            }
        }
        return true;
    }
    return false;
}
void sensors_core_get_orientation(float* xz) {
    if (!sysmon_is_module_disable(SYSMON_MODULE_MPU6050)) {
        xz[0] = mpu6050_flt_data[0];
        xz[1] = mpu6050_flt_data[1];
    } else {
        xz[0] = 0;
        xz[1] = 0;
    }
}


void sensors_core_process(void) {
    if (!sysmon_is_module_disable(SYSMON_MODULE_PCA9555) && pca9555_is_input_changed()) {
        sensors_inputs = pca9555_read_inputs(PCA9555_GPIO_SENSOR_ALL);
    }
    
    static uint32_t errors_count = 0;
    if (!sysmon_is_module_disable(SYSMON_MODULE_MPU6050) && mpu6050_is_data_ready()) {
        float raw_data[2] = {0};
        if (mpu6050_read_data(raw_data)) {
            const float flt_factor = 0.1f;
            mpu6050_flt_data[0] = raw_data[0] * flt_factor + mpu6050_flt_data[0] * (1.0f - flt_factor);
            mpu6050_flt_data[1] = raw_data[1] * flt_factor + mpu6050_flt_data[1] * (1.0f - flt_factor);
            errors_count = 0;
        } else if (++errors_count > 10) {
            sysmon_set_error(SYSMON_I2C_ERROR);
            sysmon_disable_module(SYSMON_MODULE_MPU6050);
        }
    }
}
