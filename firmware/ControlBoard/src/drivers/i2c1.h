/// ***************************************************************************
/// @file    i2c1.h
/// @author  NeoProg
/// @brief   Interface for I2C1 driver
/// ***************************************************************************
#ifndef _I2C1_H_
#define _I2C1_H_

// Timigns value for I2C clocks = 8MHz (HSI source)
typedef enum {
    I2C1_SPEED_100KHZ  = 0x10420F13,
    I2C1_SPEED_400KHZ  = 0x00310309
} i2c1_speed_t;


extern void i2c1_init(i2c1_speed_t speed);
extern bool i2c1_read(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t* buffer, uint8_t bytes_count);
extern bool i2c1_write(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t* data, uint8_t bytes_count);


#endif // _I2C1_H_

