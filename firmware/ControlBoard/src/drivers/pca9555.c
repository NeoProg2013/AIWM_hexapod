/// ***************************************************************************
/// @file    PCA9555.c
/// @author  NeoProg
/// ***************************************************************************
#include "project-base.h"
#include "pca9555.h"
#include "i2c1.h"
#include "systimer.h"
#define PCA9555_I2C_ADDRESS                 (0x40)
#define INTERRUPT_PIN                       GPIOB, 5
        
#define PCA9555_INPUT_PORT_0_ADDR           (0x00)
#define PCA9555_INPUT_PORT_1_ADDR           (0x01)
#define PCA9555_OUTPUT_PORT_0_ADDR          (0x02)
#define PCA9555_OUTPUT_PORT_1_ADDR          (0x03)
#define PCA9555_POLARITY_PORT_0_ADDR        (0x04)
#define PCA9555_POLARITY_PORT_1_ADDR        (0x05)
#define PCA9555_CFG_PORT_0_ADDR             (0x06)
#define PCA9555_CFG_PORT_1_ADDR             (0x07)


/// ***************************************************************************
/// @brief  PCA9555 initialization
/// @return true - success, false - error
/// ***************************************************************************
bool pca9555_init(void) {
    gpio_reset           (INTERRUPT_PIN);
    gpio_set_mode        (INTERRUPT_PIN, GPIO_MODE_INPUT);
    gpio_set_output_speed(INTERRUPT_PIN, GPIO_SPEED_HIGH);
    gpio_set_pull        (INTERRUPT_PIN, GPIO_PULL_UP);
    
    // Reset all outputs
    if (!pca9555_set_outputs(0xFFFF, 0x0000)) {
        return false;
    }
    
    // Configure IO0.1 IO0.3 IO0.5 as outputs
    // Configure IO1.2 IO1.4 IO1.6 as outputs
    uint16_t cfg = make16(0xAB, 0xD5);
    if (!i2c1_write16(PCA9555_I2C_ADDRESS, PCA9555_CFG_PORT_0_ADDR, 1, cfg)) {
        return false;
    }
    if (i2c1_read16(PCA9555_I2C_ADDRESS, PCA9555_CFG_PORT_0_ADDR, 1, false) != cfg) {
        return false;
    }
    
    // Reset INT pin
    pca9555_read_inputs(0xFFFF);
    return true;
}

/// ***************************************************************************
/// @brief  Check input data changed
/// @return true - data changed, false - no
/// ***************************************************************************
bool pca9555_is_input_changed(void) {
    return gpio_read_input(INTERRUPT_PIN) == 0;
}

/// ***************************************************************************
/// @brief  Read inputs
/// @param  inputs: inputs
/// @return true - pin is HIGH, false - pin is LOW
/// ***************************************************************************
uint16_t pca9555_read_inputs(uint16_t inputs) {
    return i2c1_read16(PCA9555_I2C_ADDRESS, PCA9555_INPUT_PORT_0_ADDR, 1, false) & inputs;
}

/// ***************************************************************************
/// @brief  Set outputs state
/// @param  outputs: outputs
/// @param  new_states: new outputs state
/// @return true - pin is HIGH, false - pin is LOW
/// ***************************************************************************
bool pca9555_set_outputs(uint16_t outputs, uint16_t new_states) {
    uint16_t cur_states = i2c1_read16(PCA9555_I2C_ADDRESS, PCA9555_OUTPUT_PORT_0_ADDR, 1, false); // TODO: Need remove
    cur_states &= ~outputs;   // Reset requested outputs
    cur_states |= new_states; // Apply new outputs state
    return i2c1_write16(PCA9555_I2C_ADDRESS, PCA9555_OUTPUT_PORT_0_ADDR, 1, cur_states);
}
