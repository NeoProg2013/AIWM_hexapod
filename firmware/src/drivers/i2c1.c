//  ***************************************************************************
/// @file    i2c1.c
/// @author  NeoProg
//  ***************************************************************************
#include "i2c1.h"
#include "stm32f373xc.h"
#include "systimer.h"

#define I2C_SCL_PIN                     (8) // PB8
#define I2C_SDA_PIN                     (9) // PB9

#define I2C_WAIT_TIMEOUT_VALUE          (10) // ms


static bool send_internal_address(uint32_t internal_address, uint8_t internal_address_size);
static bool wait_set_bit(volatile uint32_t* reg, uint32_t mask);
static bool wait_clear_bit(volatile uint32_t* reg, uint32_t mask);


//  ***************************************************************************
/// @brief  I2C initialization
/// @param  speed: I2C speed. @Ref i2c_speed_t
/// @return none
//  ***************************************************************************
void i2c1_init(i2c_speed_t speed) {

    // Reset I2C1 registers
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

    // Configure I2C
    I2C1->TIMINGR = speed;

    // Send 9 pulses on SCL
    GPIOB->MODER   |=  (0x01 << (I2C_SCL_PIN * 2));         // Output
    GPIOB->OTYPER  |=  (0x01 << (I2C_SCL_PIN * 1));         // Open drain
    GPIOB->OSPEEDR |=  (0x03 << (I2C_SCL_PIN * 2));         // High speed
    GPIOB->PUPDR   &= ~(0x03 << (I2C_SCL_PIN * 2));         // Disable pull
    for (uint32_t i = 0; i < 10; ++i) {
        GPIOB->BRR = 0x01 << I2C_SCL_PIN;
        delay_ms(1);
        GPIOB->BSRR = 0x01 << I2C_SCL_PIN;
        delay_ms(1);
    }
    GPIOB->MODER   &= ~(0x03 << (I2C_SCL_PIN * 2));
    
    // Setup SCL pin
    GPIOB->MODER   |=  (0x02 << (I2C_SCL_PIN * 2));         // Alternate function
    GPIOB->AFR[1]  |=  (0x04 << (I2C_SCL_PIN * 4 - 32));    // AF4
    
    // Setup SDA pin
    GPIOB->MODER   |=  (0x02 << (I2C_SDA_PIN * 2));         // Alternate function
    GPIOB->OTYPER  |=  (0x01 << (I2C_SDA_PIN * 1));         // Open drain
    GPIOB->OSPEEDR |=  (0x03 << (I2C_SDA_PIN * 2));         // High speed
    GPIOB->PUPDR   &= ~(0x03 << (I2C_SDA_PIN * 2));         // Disable pull
    GPIOB->AFR[1]  |=  (0x04 << (I2C_SDA_PIN * 4 - 32));    // AF4
    
    
}

//  ***************************************************************************
/// @brief  Read data from I2C device
/// @param  i2c_address: device address
/// @param  internal_address: device internal register address
/// @param  buffer: pointer to buffer
/// @param  bytes_count: bytes count for read
/// @return true - success, false - error
//  ***************************************************************************
bool i2c1_read(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t* buffer, uint8_t bytes_count) {

    bool result = true;

    // Set basic I2C configuration
    I2C1->CR1 |= I2C_CR1_PE;
    I2C1->CR2  = i2c_address;
    
    // Send internal address
    // Configure bytes count for send and send START condition
    I2C1->CR2 |= (internal_address_size << I2C_CR2_NBYTES_Pos) | I2C_CR2_START;
    if (send_internal_address(internal_address, internal_address_size) == false) {
        result = false;
        goto _i2c_read_operation_end;
    }

    // Read data from device
    I2C1->CR2 &= ~I2C_CR2_NBYTES;
    I2C1->CR2 |= I2C_CR2_RD_WRN | (bytes_count << I2C_CR2_NBYTES_Pos) | I2C_CR2_START;
    for (uint32_t i = 0; i < bytes_count; ++i) {

        // Wait RX register is not empty event
        if (wait_set_bit(&I2C1->ISR, I2C_ISR_RXNE) == false) {
            result = false;
            goto _i2c_read_operation_end;
        }

        // Read byte
        *buffer = I2C1->RXDR;
        ++buffer;
    }

    _i2c_read_operation_end:

    // Send STOP condition
    I2C1->CR2 |= I2C_CR2_STOP;
    if (wait_clear_bit(&I2C1->CR2, I2C_CR2_STOP) == false) {
        result = false;
    }

    // Disable I2C
    I2C1->CR1 &= ~I2C_CR1_PE;
    return result;
}

//  ***************************************************************************
/// @brief  Write data to I2C device
/// @param  i2c_address: device address
/// @param  internal_address: device internal register address
/// @param  data: pointer to buffer
/// @param  bytes_count: bytes count for write
/// @return true - success, false - error
//  ***************************************************************************
bool i2c1_write(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t* data, uint8_t bytes_count) {

    bool result = true;

    // Set basic I2C configuration
    I2C1->CR1 |= I2C_CR1_PE;
    I2C1->CR2  = i2c_address;
    
    // Configure bytes count for send and send START condition
    I2C1->CR2 |= ((internal_address_size + bytes_count) << I2C_CR2_NBYTES_Pos) | I2C_CR2_START;
    if (send_internal_address(internal_address, internal_address_size) == false) {
        result = false;
        goto _i2c_write_operation_end;
    }

    // Send data
    for (uint32_t i = 0; i < bytes_count; ++i) {

        I2C1->TXDR = *data;
        ++data;
        
        // Wait TX register empty event
        if (wait_set_bit(&I2C1->ISR, I2C_ISR_TXE) == false) {
            result = false;
            goto _i2c_write_operation_end;
        }
    }

    _i2c_write_operation_end:

    // Send STOP condition
    I2C1->CR2 |= I2C_CR2_STOP;
    if (wait_clear_bit(&I2C1->CR2, I2C_CR2_STOP) == false) {
        result = false;
    }

    // Disable I2C
    I2C1->CR1 &= ~I2C_CR1_PE;
    return result;
}





//  ***************************************************************************
/// @brief  Send internal address data
/// @note   Send internal address as MSB first
/// @param  internal_address: internal address
/// @param  internal_address_size: internal address size
/// @return true - success, false - timeout
//  ***************************************************************************
static bool send_internal_address(uint32_t internal_address, uint8_t internal_address_size) {

    uint8_t* ptr = (uint8_t*)&internal_address;
    ptr += internal_address_size - 1; // Go to MSB
    for (uint32_t i = 0; i < internal_address_size; ++i) {

        // Wait TX register empty event
        if (wait_set_bit(&I2C1->ISR, I2C_ISR_TXE) == false) {
            return false;
        }

        // Send byte
        I2C1->TXDR = *ptr;
        --ptr;
    }
    
    // Wait TX register empty event
    if (wait_set_bit(&I2C1->ISR, I2C_ISR_TXE) == false) {
        return false;
    }
    return true;
}

//  ***************************************************************************
/// @brief  Wait bit set in register with timeout
/// @param  reg: register address
/// @param  mask: mask
/// @return true - success, false - timeout
//  ***************************************************************************
static bool wait_set_bit(volatile uint32_t* reg, uint32_t mask) {

    uint64_t start_time = get_time_ms();
    while ((*reg & mask) == 0) {
        
        if (get_time_ms() - start_time > I2C_WAIT_TIMEOUT_VALUE || (I2C1->ISR & (I2C_ISR_OVR | I2C_ISR_ARLO | I2C_ISR_BERR | I2C_ISR_NACKF)) ) {
            return false;
        }
    }
    return true;
}

//  ***************************************************************************
/// @brief  Wait bit clear in register with timeout
/// @param  reg: register address
/// @param  mask: mask
/// @return true - success, false - timeout
//  ***************************************************************************
static bool wait_clear_bit(volatile uint32_t* reg, uint32_t mask) {

    uint64_t start_time = get_time_ms();
    while (*reg & mask) {
        
        if (get_time_ms() - start_time > I2C_WAIT_TIMEOUT_VALUE || (I2C1->ISR & (I2C_ISR_OVR | I2C_ISR_ARLO | I2C_ISR_BERR | I2C_ISR_NACKF)) ) {
            return false;
        }
    }
    return true;
}
