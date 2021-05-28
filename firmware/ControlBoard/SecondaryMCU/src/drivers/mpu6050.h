//  ***************************************************************************
/// @file    mpu6050.h
/// @author  NeoProg
/// @brief   MPU6050 driver
//  ***************************************************************************
#ifndef _MPU6050_H_
#define _MPU6050_H_

#define MPU6050_DRIVER_NO_ERROR					(0x00)
#define MPU6050_DRIVER_BUSY						(0x01)
#define MPU6050_DRIVER_ERROR					(0x02)


/**************************************************************************
* @brief	MPU6050 initialization
* @note		Write firmware for DMP
* @return   true - initialize success, false - initialize fail
**************************************************************************/
bool mpu6050_init(void);

/**************************************************************************
* @brief	MPU6050 calibration
* @return   true - success, false - error
**************************************************************************/
bool mpu6050_calibration(void);

/**************************************************************************
* @brief	Function for DMP start\stop
* @param	is_enable: true - DMP start, false - DMP stop
* @return	true - success, false - error
**************************************************************************/
bool mpu6050_set_state(bool is_enable);

/**************************************************************************
* @brief	Read data from MPU6050
* @param	XY: buffer for angles
* @return   true - success, false - error
**************************************************************************/
bool mpu6050_read_data(float* xy_angles, bool* is_ready);


#endif /* __MPU6050_H__ */