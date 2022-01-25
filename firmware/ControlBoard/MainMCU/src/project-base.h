//  ***************************************************************************
/// @file    project-base.c
/// @author  NeoProg
/// @brief   Contain basic information and declarations. Should be include to all project files
//  ***************************************************************************
#ifndef _PROJECT_BASE_H_
#define _PROJECT_BASE_H_
#include "stm32f373xc.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


#define SYSTEM_CLOCK_FREQUENCY              (72000000)
#define AHB_CLOCK_FREQUENCY                 (72000000)
#define APB1_CLOCK_FREQUENCY                (36000000)
#define APB2_CLOCK_FREQUENCY                (72000000)


#define DEBUG_TP1_PIN                       GPIOC, 6    // PC6
#define DEBUG_TP2_PIN                       GPIOA, 8    // PA8
#define DEBUG_TP3_PIN                       GPIOA, 12   // PA12

#define DEBUG_TP1_PIN_SET                   gpio_set(DEBUG_TP1_PIN)
#define DEBUG_TP1_PIN_CLR                   gpio_reset(DEBUG_TP1_PIN)
#define DEBUG_TP1_PIN_TOGGLE                gpio_toggle(DEBUG_TP1_PIN)
                                                                                                                          
#define DEBUG_TP3_PIN_SET                   gpio_set(DEBUG_TP3_PIN)
#define DEBUG_TP3_PIN_CLR                   gpio_reset(DEBUG_TP3_PIN)
#define DEBUG_TP3_PIN_TOGGLE                gpio_toggle(DEBUG_TP3_PIN)

#define USART1_TX_BUFFER_SIZE               (3072)

#define TIM17_IRQ_PRIORITY                  (0)        // 18-channels PWM driver
#define USART2_IRQ_PRIORITY                 (2)        // SWLP communication
#define USART1_IRQ_PRIORITY                 (7)        // CLI communication
#define USART3_IRQ_PRIORITY                 (3)        // Camera communication
#define I2C2_IRQ_PRIORITY                   (4)        // Display communication
#define ADC_IRQ_PRIORITY                    (4)        // Battery voltage measurements





#define GPIO_MODE_INPUT                     (0x00)
#define GPIO_MODE_OUTPUT                    (0x01)
#define GPIO_MODE_AF                        (0x02)
#define GPIO_MODE_ANALOG                    (0x03)
static inline void gpio_set_mode(GPIO_TypeDef* port, uint32_t pin, uint32_t mode) {
    port->MODER &= ~(0x03u << (pin * 2u)); // Clear
    port->MODER |=  (mode  << (pin * 2u)); // Set
}

#define GPIO_TYPE_PUSH_PULL                 (0x00)
#define GPIO_TYPE_OPEN_DRAIN                (0x01)
static inline void gpio_set_output_type(GPIO_TypeDef* port, uint32_t pin, uint32_t type) {
    port->OTYPER &= ~(0x01u << (pin * 1u)); // Clear
    port->OTYPER |=  (type  << (pin * 1u)); // Set
}

#define GPIO_SPEED_LOW                      (0x00)
#define GPIO_SPEED_MEDIUM                   (0x01)
#define GPIO_SPEED_HIGH                     (0x01)
static inline void gpio_set_output_speed(GPIO_TypeDef* port, uint32_t pin, uint32_t speed) {
    port->OSPEEDR &= ~(0x03u << (pin * 2u)); // Clear
    port->OSPEEDR |=  (speed << (pin * 2u)); // Set
}

#define GPIO_PULL_NO                        (0x00)
#define GPIO_PULL_UP                        (0x01)
#define GPIO_PULL_DOWN                      (0x02)
static inline void gpio_set_pull(GPIO_TypeDef* port, uint32_t pin, uint32_t pull) {
    port->PUPDR &= ~(0x03u << (pin * 2u)); // Clear
    port->PUPDR |=  (pull  << (pin * 2u)); // Set
}

static inline void gpio_set_af(GPIO_TypeDef* port, uint32_t pin, uint32_t af) {
    if (pin < 8) {
        port->AFR[0] &= ~(0x0Fu << (pin * 4u)); // Clear
        port->AFR[0] |=  (af    << (pin * 4u)); // Set
    } else {
        port->AFR[1] &= ~(0x0Fu << (pin * 4u - 32u)); // Clear
        port->AFR[1] |=  (af    << (pin * 4u - 32u)); // Set
    }
}

static inline void gpio_set(GPIO_TypeDef* port, uint32_t pin) {
    port->BSRR |= 0x01u << pin;
}
static inline void gpio_reset(GPIO_TypeDef* port, uint32_t pin) {
    port->BRR |= 0x01u << pin;
}
static inline void gpio_toggle(GPIO_TypeDef* port, uint32_t pin) {
    port->ODR ^= 0x01u << pin;
}
static inline bool gpio_read_input(GPIO_TypeDef* port, uint32_t pin) {
    return port->IDR & (0x01u << pin);
}
static inline bool gpio_read_output(GPIO_TypeDef* port, uint32_t pin) {
    return port->ODR & (0x01u << pin);
}



#endif // _PROJECT_BASE_H_
