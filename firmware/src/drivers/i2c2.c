//  ***************************************************************************
/// @file    i2c2.c
/// @author  NeoProg
//  ***************************************************************************
#include "i2c2.h"
#include "stm32f373xc.h"
#include "project_base.h"
#include "systimer.h"

#define I2C_SCL_PIN                     (6) // PF6
#define I2C_SDA_PIN                     (7) // PF7

#define I2C_MAX_TX_BYTE_TIME            (2) // ms


static bool is_driver_error = false;
static uint32_t async_operation_timeout_value = 0;


static bool send_internal_address(uint32_t internal_address, uint8_t internal_address_size);
static bool wait_set_bit(volatile uint32_t* reg, uint32_t mask);
static void disable_i2c(void);


//  ***************************************************************************
/// @brief  I2C initialization
/// @param  speed: I2C speed. @Ref i2c_speed_t
/// @return none
//  ***************************************************************************
void i2c2_init(i2c_speed_t speed) {
    
    //
    // Setup GPIO
    //
    // Send 9 pulses on SCL
    GPIOF->MODER   |=  (0x01 << (I2C_SCL_PIN * 2));     // Output mode
    GPIOF->OTYPER  |=  (0x01 << (I2C_SCL_PIN * 1));     // Open drain
    GPIOF->OSPEEDR |=  (0x03 << (I2C_SCL_PIN * 2));     // High speed
    GPIOF->PUPDR   &= ~(0x03 << (I2C_SCL_PIN * 2));     // Disable pull
    for (uint32_t i = 0; i < 10; ++i) {
        GPIOF->BRR = 0x01 << I2C_SCL_PIN;
        delay_ms(1);
        GPIOF->BSRR = 0x01 << I2C_SCL_PIN;
        delay_ms(1);
    }
    GPIOF->MODER   &= ~(0x03 << (I2C_SCL_PIN * 2));
    
    // Setup SCL pin (PB8)
    GPIOF->MODER   |=  (0x02 << (I2C_SCL_PIN * 2));     // Alternate function mode
    GPIOF->AFR[0]  |=  (0x04 << (I2C_SCL_PIN * 4));     // AF4
    
    // Setup SDA pin (PB9)
    GPIOF->MODER   |=  (0x02 << (I2C_SDA_PIN * 2));     // Alternate function mode
    GPIOF->OTYPER  |=  (0x01 << (I2C_SDA_PIN * 1));     // Open drain
    GPIOF->OSPEEDR |=  (0x03 << (I2C_SDA_PIN * 2));     // High speed
    GPIOF->PUPDR   &= ~(0x03 << (I2C_SDA_PIN * 2));     // Disable pull
    GPIOF->AFR[0]  |=  (0x04 << (I2C_SDA_PIN * 4));     // AF4
    
    
    //
    // Setup I2C2
    //
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C2RST;

    // Configure I2C
    I2C2->TIMINGR = speed;
    NVIC_EnableIRQ(I2C2_EV_IRQn);
    NVIC_SetPriority(I2C2_EV_IRQn, I2C2_IRQ_PRIORITY);
    NVIC_EnableIRQ(I2C2_ER_IRQn);
    NVIC_SetPriority(I2C2_ER_IRQn, I2C2_IRQ_PRIORITY);
    
    // Configure TX DMA
    DMA1_Channel4->CCR  &= ~DMA_CCR_EN;
    DMA1_Channel4->CCR   = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TEIE;
    DMA1_Channel4->CPAR  = (uint32_t)(&I2C2->TXDR);
    DMA1_Channel4->CMAR  = 0;
    DMA1_Channel4->CNDTR = 0;
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);
    NVIC_SetPriority(DMA1_Channel4_IRQn, I2C2_IRQ_PRIORITY);
}

//  ***************************************************************************
/// @brief  Async write data to I2C device
/// @note   Internal address send using sync mode
/// @param  i2c_address: device address
/// @param  internal_address: device internal register address
/// @param  data: pointer to buffer
/// @param  bytes_count: bytes count for write
/// @return true - operation started, false - error
//  ***************************************************************************
bool i2c2_async_write(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t* data, uint8_t bytes_count) {

    if (i2c2_is_async_operation_completed() == false) {
        return false;
    }

    // Set sync I2C configuration
    I2C2->CR1 = I2C_CR1_PE;
    I2C2->CR2 = i2c_address | ((internal_address_size + bytes_count) << I2C_CR2_NBYTES_Pos) | I2C_CR2_START;
    
    // Send internal address (MSB first)
    if (send_internal_address(internal_address, internal_address_size) == false) {
        disable_i2c();
        return false;
    }
    
    // Set async I2C configuration
    I2C2->ICR = 0xFFFFFFFF;
    I2C2->CR1 |= I2C_CR1_TCIE | I2C_CR1_NACKIE | I2C_CR1_ERRIE | I2C_CR1_TXDMAEN;
    DMA1_Channel4->CMAR  = (uint32_t)data;
    DMA1_Channel4->CNDTR = bytes_count;
    DMA1_Channel4->CCR  |= DMA_CCR_EN;

    is_driver_error = false;
    async_operation_timeout_value = get_time_ms() + bytes_count * I2C_MAX_TX_BYTE_TIME;
    return true;
}

//  ***************************************************************************
/// @brief  Write data to I2C device
/// @param  i2c_address: device address
/// @param  internal_address: device internal register address
/// @param  data: pointer to buffer
/// @param  bytes_count: bytes count for write
/// @return true - success, false - error
//  ***************************************************************************
bool i2c2_write(uint8_t i2c_address, uint32_t internal_address, uint8_t internal_address_size, uint8_t* data, uint8_t bytes_count) {
    
    if (i2c2_async_write(i2c_address, internal_address, internal_address_size, data, bytes_count) == false) {
        return false;
    }
    while (i2c2_is_async_operation_completed() == false);
    return i2c2_get_async_operation_result();
}

//  ***************************************************************************
/// @brief  Get async operation state
/// @return true - operation completed, false - operation in progress
//  ***************************************************************************
bool i2c2_is_async_operation_completed(void) {
    
    if (I2C2->ISR & I2C_ISR_BUSY) {
        
        if (get_time_ms() > async_operation_timeout_value) {
            disable_i2c();
        }
        return false;
    }
    return true;
}

//  ***************************************************************************
/// @brief  Get async operation status
/// @return true - success, false - error
//  ***************************************************************************
bool i2c2_get_async_operation_result(void) {
    return is_driver_error == false;
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

//  ***************************************************************************
/// @brief  Wait bit set in register with timeout
/// @param  reg: register address
/// @param  mask: mask
/// @return true - success, false - timeout
//  ***************************************************************************
#define I2C_WAIT_TIMEOUT_VALUE          (5) // ms
static bool wait_set_bit(volatile uint32_t* reg, uint32_t mask) {

    uint64_t start_time = get_time_ms();
    do {
        if ((get_time_ms() - start_time > I2C_WAIT_TIMEOUT_VALUE) || (I2C2->ISR & (I2C_ISR_OVR | I2C_ISR_ARLO | I2C_ISR_BERR | I2C_ISR_NACKF)) ) {
            return false;
        }
    } 
    while ((*reg & mask) == 0);
    return true;
}

//  ***************************************************************************
/// @brief  Disable I2C
/// @return none
//  ***************************************************************************
static void disable_i2c(void) {
    
    // Send stop condition
    I2C2->CR2 |= I2C_CR2_STOP;
    for (uint32_t i = 0; (i < 1000) && (I2C2->CR2 & I2C_CR2_STOP); ++i);
    
    // Disable I2C
    DMA1_Channel4->CCR &= ~DMA_CCR_EN;
    I2C2->CR1 = 0;
}





//  ***************************************************************************
/// @brief  I2C2 event ISR
/// @param  none
/// @return none
//  ***************************************************************************
void I2C2_EV_IRQHandler(void) {
    
    uint32_t status = I2C2->ISR;
    I2C2->ICR = 0xFFFFFFFF; // Clear flags

    if (status & I2C_ISR_NACKF) {
        is_driver_error = true;
    }
    else if (status & I2C_ISR_TC) {
        is_driver_error = false;
    }
    disable_i2c();
}

//  ***************************************************************************
/// @brief  I2C2 error ISR
/// @param  none
/// @return none
//  ***************************************************************************
void I2C2_ER_IRQHandler(void) {
    
    uint32_t status = I2C2->ISR;
    I2C2->ICR = 0xFFFFFFFF; // Clear flags

    if (status & (I2C_ISR_ARLO | I2C_ISR_BERR | I2C_ISR_OVR | I2C_ISR_NACKF)) {
        is_driver_error = true;
        disable_i2c();
    }
}
