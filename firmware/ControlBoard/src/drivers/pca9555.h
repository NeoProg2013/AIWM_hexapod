/// ***************************************************************************
/// @file    PCA9555.h
/// @author  NeoProg
/// @brief   PCA9555 driver
/// ***************************************************************************
#ifndef _PCA9555_H_
#define _PCA9555_H_

typedef enum {
    // Outputs
    PCA9555_GPIO_LED_LEFT_1  = 0x0002, // IO01
    PCA9555_GPIO_LED_LEFT_2  = 0x0008, // IO03
    PCA9555_GPIO_LED_LEFT_3  = 0x0020, // IO05
    PCA9555_GPIO_LED_RIGHT_1 = 0x4000, // IO16
    PCA9555_GPIO_LED_RIGHT_2 = 0x1000, // IO14
    PCA9555_GPIO_LED_RIGHT_3 = 0x0400, // IO12
    PCA9555_GPIO_LED_ALL = PCA9555_GPIO_LED_LEFT_1  | PCA9555_GPIO_LED_LEFT_2  | PCA9555_GPIO_LED_LEFT_3 | 
                           PCA9555_GPIO_LED_RIGHT_1 | PCA9555_GPIO_LED_RIGHT_2 | PCA9555_GPIO_LED_RIGHT_3,
                      
    // Inputs
    PCA9555_GPIO_SENSOR_LEFT_1  = 0x0001, // IO00
    PCA9555_GPIO_SENSOR_LEFT_2  = 0x0004, // IO02
    PCA9555_GPIO_SENSOR_LEFT_3  = 0x0010, // IO04
    PCA9555_GPIO_SENSOR_RIGHT_1 = 0x8000, // IO17
    PCA9555_GPIO_SENSOR_RIGHT_2 = 0x2000, // IO15
    PCA9555_GPIO_SENSOR_RIGHT_3 = 0x0800, // IO13
    PCA9555_GPIO_SENSOR_ALL = PCA9555_GPIO_SENSOR_LEFT_1  | PCA9555_GPIO_SENSOR_LEFT_2  | PCA9555_GPIO_SENSOR_LEFT_3 | 
                              PCA9555_GPIO_SENSOR_RIGHT_1 | PCA9555_GPIO_SENSOR_RIGHT_2 | PCA9555_GPIO_SENSOR_RIGHT_3,             
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
/// @param  states: new outputs state
/// @return true - pin is HIGH, false - pin is LOW
/// ***************************************************************************
extern bool pca9555_set_outputs(uint16_t states);

#endif // _PCA9555_H_
