//  ***************************************************************************
/// @file    i2c2.h
/// @author  NeoProg
/// @brief   Interface for I2C2 driver
//  ***************************************************************************
#ifndef _I2C2_H_
#define _I2C2_H_

#include <stdint.h>
#include <stdbool.h>

// Timigns value for I2C clocks = 8MHz (HSI source)
typedef enum {
    I2C_SPEED_100KHZ  = 0x10420F13,
    I2C_SPEED_400KHZ  = 0x00310309
} i2c_speed_t;


extern void i2c2_init(i2c_speed_t speed);
extern bool i2c2_write(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t* data, uint8_t bytes_count);
extern bool i2c2_async_write(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t* data, uint8_t bytes_count);
extern bool i2c2_is_async_operation_completed(void);
extern bool i2c2_get_async_operation_result(void);


#endif // _I2C2_H_

