/// ***************************************************************************
/// @file    PCA9555.h
/// @author  NeoProg
/// @brief   PCA9555 driver
/// ***************************************************************************
#ifndef _PCA9555_H_
#define _PCA9555_H_

typedef enum {
    PCA9555_GPIO_LED_RIGHT_1 = 0x0020, // IO05
    PCA9555_GPIO_LED_RIGHT_2 = 0x0008, // IO03
    PCA9555_GPIO_LED_RIGHT_3 = 0x0002, // IO01
    PCA9555_GPIO_LED_LEFT_1  = 0x0400, // IO12
    PCA9555_GPIO_LED_LEFT_2  = 0x1000, // IO14
    PCA9555_GPIO_LED_LEFT_3  = 0x4002, // IO16
} pca9555_gpio_t;

/// ***************************************************************************
/// @brief  PCA9555 initialization
/// @return true - success, false - error
/// ***************************************************************************
extern bool pca9555_init(void);

/// ***************************************************************************
/// @brief  Check input data changed
/// @return true - data changed, false - no
/// ***************************************************************************
extern bool pca9555_is_input_changed(void);

/// ***************************************************************************
/// @brief  Read inputs
/// @param  inputs: inputs
/// @return true - pin is HIGH, false - pin is LOW
/// ***************************************************************************
extern uint16_t pca9555_read_inputs(uint16_t inputs);

/// ***************************************************************************
/// @brief  Set outputs state
/// @param  outputs: outputs
/// @param  new_states: new outputs state
/// @return true - pin is HIGH, false - pin is LOW
/// ***************************************************************************
extern bool pca9555_set_outputs(uint16_t outputs, uint16_t new_states);

#endif // _PCA9555_H_
