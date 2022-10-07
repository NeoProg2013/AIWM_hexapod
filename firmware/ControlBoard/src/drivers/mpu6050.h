//  ***************************************************************************
/// @file    mpu6050.h
/// @author  NeoProg
/// @brief   MPU6050 driver
//  ***************************************************************************
#ifndef _MPU6050_H_
#define _MPU6050_H_


/// ***************************************************************************
/// @brief  MPU6050 initialization
/// @note   Write firmware for DMP
/// @return true - initialize success, false - initialize fail
/// ***************************************************************************
bool mpu6050_init(void);

/// ***************************************************************************
/// @brief  Function for DMP start\stop
/// @param  is_enable: true - DMP start, false - DMP stop
/// @return true - success, false - error
/// ***************************************************************************
bool mpu6050_set_state(bool is_enable);

/// ***************************************************************************
/// @brief  MPU6050 calibration
/// @return true - success, false - error
/// ***************************************************************************
bool mpu6050_calibration(void);

/// ***************************************************************************
/// @brief  Check data ready
/// @return true - data is ready, false - no
/// ***************************************************************************
bool mpu6050_is_data_ready(void);

/// ***************************************************************************
/// @brief  Read data from MPU6050
/// @param  XY: buffer for angles
/// @return true - success, false - error
/// ***************************************************************************
bool mpu6050_read_data(float* xy_angles);


#endif /* __MPU6050_H__ */