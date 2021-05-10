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
* @retval	true - initialize success, false - initialize fail
**************************************************************************/
bool mpu6050_init(void);

/**************************************************************************
* @brief	Function for DMP start
* @retval	true - success, false - error
**************************************************************************/
bool mpu6050_start(void);

/**************************************************************************
* @brief	Function for DMP stop
* @retval	true - success, false - error
**************************************************************************/
bool mpu6050_stop(void);

/**************************************************************************
* @brief	Function for check data ready
* @retval	true - data ready, false - data not ready or error
**************************************************************************/
bool mpu6050_is_data_ready(bool* is_ready);

/**************************************************************************
* @brief	Function for get data (XYZ) from MPU6050
* @note		This function use I2C async mode for read FIFO packet
* @param	XY: buffer for angles
**************************************************************************/
bool mpu6050_read_data(float* xy_angles);

#endif /* __MPU6050_H__ */