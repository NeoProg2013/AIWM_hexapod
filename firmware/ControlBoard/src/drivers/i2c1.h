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


/// ***************************************************************************
/// @brief  I2C initialization
/// @param  speed: I2C speed. @Ref i2c_speed_t
/// ***************************************************************************
extern void i2c1_init(i2c1_speed_t speed);

/// ***************************************************************************
/// @brief  Read data from I2C device
/// @param  i2c_address: device address
/// @param  internal_address: device internal register address
/// @param  buffer: pointer to buffer
/// @param  bytes_count: bytes count for read
/// @return true - success, false - error
/// ***************************************************************************
extern bool i2c1_read(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t* buffer, uint8_t bytes_count);

/// ***************************************************************************
/// @brief  Wrappers for read function
/// @param  i2c_address: device address
/// @param  internal_address: device internal register address
/// @param  is_msbf: true - most significant byte first, false - least significant byte first
/// @return readed value, 0 - error
/// ***************************************************************************
extern uint8_t  i2c1_read8(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size);
extern uint16_t i2c1_read16(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, bool is_msbf);

/// ***************************************************************************
/// @brief  Write data to I2C device
/// @param  i2c_address: device address
/// @param  internal_address: device internal register address
/// @param  data: data for write
/// @param  bytes_count: bytes count for write
/// @return true - success, false - error
/// ***************************************************************************
extern bool i2c1_write(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t* data, uint8_t bytes_count);

/// ***************************************************************************
/// @brief  Wrappers for write function
/// @param  i2c_address: device address
/// @param  internal_address: device internal register address
/// @param  data: data for write
/// @return true - success, false - error
/// ***************************************************************************
extern bool i2c1_write8(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t data);
extern bool i2c1_write16(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint16_t data);


#endif // _I2C1_H_

