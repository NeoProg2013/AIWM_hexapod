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
/// @param  none
/// @return true - success, false - error
/// ***************************************************************************
bool pca9555_init(void) {
    gpio_reset           (INTERRUPT_PIN);
    gpio_set_mode        (INTERRUPT_PIN, GPIO_MODE_INPUT);
    gpio_set_output_speed(INTERRUPT_PIN, GPIO_SPEED_HIGH);
    gpio_set_pull        (INTERRUPT_PIN, GPIO_PULL_UP);
    
    uint8_t wdata[2] = {0};
    uint8_t rdata[2] = {0};
    
    // Reset all outputs
    wdata[0] = 0x00;
    wdata[1] = 0x00;
    if (!i2c1_write(PCA9555_I2C_ADDRESS, PCA9555_OUTPUT_PORT_0_ADDR, 1, (uint8_t*)&wdata, 2)) return false;
    if (!i2c1_read(PCA9555_I2C_ADDRESS, PCA9555_OUTPUT_PORT_0_ADDR, 1, rdata, 2)) return false;
    if (rdata[0] != wdata[0] || rdata[1] != wdata[1]) {
        return false;
    }
    memset(wdata, 0x00, sizeof(wdata));
    memset(rdata, 0x00, sizeof(rdata));
    
    // Configure IO0.1 IO0.3 IO0.5 as outputs
    // Configure IO1.2 IO1.4 IO1.6 as outputs
    wdata[0] = 0xD5;
    wdata[1] = 0xAB;
    if (!i2c1_write(PCA9555_I2C_ADDRESS, PCA9555_CFG_PORT_0_ADDR, 1, wdata, 2)) return false;
    if (!i2c1_read(PCA9555_I2C_ADDRESS, PCA9555_CFG_PORT_0_ADDR, 1, rdata, 2)) return false;
    if (rdata[0] != wdata[0] || rdata[1] != wdata[1]) {
        return false;
    }
    memset(wdata, 0x00, sizeof(wdata));
    memset(rdata, 0x00, sizeof(rdata));
    
    return pca9555_read(rdata);
}

bool pca9555_is_input_changed(void) {
    return gpio_read_input(INTERRUPT_PIN) == 0;
}

bool pca9555_read(uint8_t* data) {
    return i2c1_read(PCA9555_I2C_ADDRESS, PCA9555_INPUT_PORT_0_ADDR, 1, data, 2);
}

bool pca9555_write(uint16_t data) {
    uint8_t wdata[2] = {(data >> 0) & 0x00FF, (data >> 8) & 0x00FF};
    uint8_t rdata[2] = {0};
    if (!i2c1_write(PCA9555_I2C_ADDRESS, PCA9555_OUTPUT_PORT_0_ADDR, 1, wdata, 2))  return false;
    if (!i2c1_read(PCA9555_I2C_ADDRESS, PCA9555_OUTPUT_PORT_0_ADDR, 1, rdata, 2)) return false;
    if (rdata[0] != wdata[0] || rdata[1] != wdata[1]) {
        return false;
    }
    return true;
}
