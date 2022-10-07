/// ***************************************************************************
/// @file    i2c2.c
/// @author  NeoProg
/// ***************************************************************************
#include "project-base.h"
#include "i2c2.h"
#include "systimer.h"
#define I2C_SCL_PIN                     GPIOF, 6
#define I2C_SDA_PIN                     GPIOF, 7
#define I2C_WAIT_TIMEOUT_VALUE          (2) // ms


static bool send_internal_address(uint32_t internal_address, uint8_t internal_address_size);
static bool wait_set_bit(volatile uint32_t* reg, uint32_t mask);
static bool wait_clear_bit(volatile uint32_t* reg, uint32_t mask);


/// ***************************************************************************
/// @brief  I2C initialization
/// @param  speed: I2C speed. @Ref i2c_speed_t
/// ***************************************************************************
void i2c2_init(i2c2_speed_t speed) {
    // Send pulses on SCL
    gpio_set_mode        (I2C_SCL_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (I2C_SCL_PIN, GPIO_TYPE_OPEN_DRAIN);
    gpio_set_output_speed(I2C_SCL_PIN, GPIO_SPEED_HIGH);
    gpio_set_pull        (I2C_SCL_PIN, GPIO_PULL_NO);
    for (uint32_t i = 0; i < 10; ++i) {
        gpio_reset(I2C_SCL_PIN);
        delay_ms(1);
        gpio_set(I2C_SCL_PIN);
        delay_ms(1);
    }
    
    // Setup SCL pin (PF6)
    gpio_set_mode(I2C_SCL_PIN, GPIO_MODE_AF);
    gpio_set_af  (I2C_SCL_PIN, 4);
    
    // Setup SDA pin (PF7)
    gpio_set_mode        (I2C_SDA_PIN, GPIO_MODE_AF);
    gpio_set_output_type (I2C_SDA_PIN, GPIO_TYPE_OPEN_DRAIN);
    gpio_set_output_speed(I2C_SDA_PIN, GPIO_SPEED_HIGH);
    gpio_set_pull        (I2C_SDA_PIN, GPIO_PULL_NO);
    gpio_set_af          (I2C_SDA_PIN, 4);
    
    // Setup I2C
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C2RST;
    I2C2->TIMINGR = speed;
}

/// ***************************************************************************
/// @brief  Read data from I2C device
/// @param  i2c_address: device address
/// @param  internal_address: device internal register address
/// @param  buffer: pointer to buffer
/// @param  bytes_count: bytes count for read
/// @return true - success, false - error
/// ***************************************************************************
bool i2c2_read(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t* buffer, uint8_t bytes_count) {
    bool result = true;

    // Set basic I2C configuration
    I2C2->CR1 |= I2C_CR1_PE;
    I2C2->CR2  = i2c_address;
    
    // Send internal address
    // Configure bytes count for send and send START condition
    I2C2->CR2 |= (internal_address_size << I2C_CR2_NBYTES_Pos) | I2C_CR2_START;
    if (!send_internal_address(internal_address, internal_address_size)) {
        result = false;
        goto _i2c_read_operation_end;
    }

    // Read data from device
    I2C2->CR2 &= ~I2C_CR2_NBYTES;
    I2C2->CR2 |= I2C_CR2_RD_WRN | (bytes_count << I2C_CR2_NBYTES_Pos) | I2C_CR2_START;
    for (uint32_t i = 0; i < bytes_count; ++i) {

        // Wait RX register is not empty event
        if (!wait_set_bit(&I2C2->ISR, I2C_ISR_RXNE)) {
            result = false;
            goto _i2c_read_operation_end;
        }

        // Read byte
        *buffer = I2C2->RXDR;
        ++buffer;
    }

    _i2c_read_operation_end:

    // Send STOP condition
    I2C2->CR2 |= I2C_CR2_STOP;
    if (!wait_clear_bit(&I2C2->CR2, I2C_CR2_STOP)) {
        result = false;
    }

    // Disable I2C
    I2C2->CR1 &= ~I2C_CR1_PE;
    return result;
}

/// ***************************************************************************
/// @brief  Wrappers for read function
/// @param  i2c_address: device address
/// @param  internal_address: device internal register address
/// @return readed value, 0 - error
/// ***************************************************************************
uint8_t i2c2_read8(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size) {
    uint8_t data = 0;
    if (!i2c2_read(i2c_address, internal_address, internal_address_size, &data, 1)) {
        return 0;
    }
    return data;
}
uint16_t i2c2_read16(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, bool is_msbf) {
    uint8_t data[2] = {0};
    if (!i2c2_read(i2c_address, internal_address, internal_address_size, data, 2)) {
        return 0;
    }
    if (is_msbf) {
        return make16(data[0], data[1]);
    }
    return make16(data[1], data[0]);
}

/// ***************************************************************************
/// @brief  Write data to I2C device
/// @param  i2c_address: device address
/// @param  internal_address: device internal register address
/// @param  data: data for write
/// @param  bytes_count: bytes count for write
/// @return true - success, false - error
/// ***************************************************************************
bool i2c2_write(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t* data, uint8_t bytes_count) {
    bool result = true;

    // Set basic I2C configuration
    I2C2->CR1 |= I2C_CR1_PE;
    I2C2->CR2  = i2c_address;
    
    // Configure bytes count for send and send START condition
    I2C2->CR2 |= ((internal_address_size + bytes_count) << I2C_CR2_NBYTES_Pos) | I2C_CR2_START;
    if (!send_internal_address(internal_address, internal_address_size)) {
        result = false;
        goto _i2c_write_operation_end;
    }

    // Send data
    for (uint32_t i = 0; i < bytes_count; ++i) {
        I2C2->TXDR = *data;
        ++data;
        
        // Wait TX register empty event
        if (!wait_set_bit(&I2C2->ISR, I2C_ISR_TXE)) {
            result = false;
            goto _i2c_write_operation_end;
        }
    }

    _i2c_write_operation_end:

    // Send STOP condition
    I2C2->CR2 |= I2C_CR2_STOP;
    if (!wait_clear_bit(&I2C2->CR2, I2C_CR2_STOP)) {
        result = false;
    }

    // Disable I2C
    I2C2->CR1 &= ~I2C_CR1_PE;
    return result;
}

/// ***************************************************************************
/// @brief  Wrappers for write function
/// @param  i2c_address: device address
/// @param  internal_address: device internal register address
/// @param  data: data for write
/// @return true - success, false - error
/// ***************************************************************************
bool i2c2_write8(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t data) {
    return i2c2_write(i2c_address, internal_address, internal_address_size, &data, 1);
}
bool i2c2_write16(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint16_t data) {
    return i2c2_write(i2c_address, internal_address, internal_address_size, (uint8_t*)&data, 2);
}





/// ***************************************************************************
/// @brief  Send internal address data
/// @note   Send internal address as MSB first
/// @param  internal_address: internal address
/// @param  internal_address_size: internal address size
/// @return true - success, false - timeout
/// ***************************************************************************
static bool send_internal_address(uint32_t internal_address, uint8_t internal_address_size) {
    uint8_t* ptr = (uint8_t*)&internal_address;
    ptr += internal_address_size - 1; // Go to MSB
    for (uint32_t i = 0; i < internal_address_size; ++i) {

        // Wait TX register empty event
        if (wait_set_bit(&I2C2->ISR, I2C_ISR_TXE) == false) {
            return false;
        }

        // Send byte
        I2C2->TXDR = *ptr;
        --ptr;
    }
    
    // Wait TX register empty event
    if (wait_set_bit(&I2C2->ISR, I2C_ISR_TXE) == false) {
        return false;
    }
    return true;
}

/// ***************************************************************************
/// @brief  Wait bit set in register with timeout
/// @param  reg: register address
/// @param  mask: mask
/// @return true - success, false - timeout
/// ***************************************************************************
static bool wait_set_bit(volatile uint32_t* reg, uint32_t mask) {
    uint64_t start_time = get_time_ms();
    do {
        if (get_time_ms() - start_time > I2C_WAIT_TIMEOUT_VALUE) {
            return false; // Timeout
        }
        if (I2C2->ISR & (I2C_ISR_OVR | I2C_ISR_ARLO | I2C_ISR_BERR | I2C_ISR_NACKF)) {
            return false; // I2C bus error
        }
    } while ((*reg & mask) == 0);
    return true;
}


/// ***************************************************************************
/// @brief  Wait bit clear in register with timeout
/// @param  reg: register address
/// @param  mask: mask
/// @return true - success, false - timeout
/// ***************************************************************************
static bool wait_clear_bit(volatile uint32_t* reg, uint32_t mask) {
    uint64_t start_time = get_time_ms();
    do {
        if (get_time_ms() - start_time > I2C_WAIT_TIMEOUT_VALUE) {
            return false; // Timeout
        }
        if (I2C2->ISR & (I2C_ISR_OVR | I2C_ISR_ARLO | I2C_ISR_BERR | I2C_ISR_NACKF)) {
            return false; // I2C bus error
        }
    } while (*reg & mask);
    return true;
}
