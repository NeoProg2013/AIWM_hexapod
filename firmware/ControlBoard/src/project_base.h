//  ***************************************************************************
/// @file    project_base.c
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


#define DEBUG_TP1_PIN                       (9)     // PC9
#define DEBUG_TP3_PIN                       (11)    // PA11

#define DEBUG_TP1_PIN_SET                   (GPIOC->BSRR = 0x01 << DEBUG_TP1_PIN)
#define DEBUG_TP1_PIN_CLR                   (GPIOC->BRR  = 0x01 << DEBUG_TP1_PIN)
#define DEBUG_TP1_PIN_TOGGLE                (GPIOC->ODR ^= 0x01 << DEBUG_TP1_PIN)
                                                                                                                          
#define DEBUG_TP3_PIN_SET                   (GPIOA->BSRR = 0x01 << DEBUG_TP3_PIN)                                         
#define DEBUG_TP3_PIN_CLR                   (GPIOA->BRR  = 0x01 << DEBUG_TP3_PIN)                                         
#define DEBUG_TP3_PIN_TOGGLE                (GPIOA->ODR ^= 0x01 << DEBUG_TP3_PIN)


#define TIM17_IRQ_PRIORITY                  (0)        // 18-channels PWM driver
#define USART2_IRQ_PRIORITY                 (2)        // SWLP communication
#define USART1_IRQ_PRIORITY                 (7)        // CLI communication
#define USART3_IRQ_PRIORITY                 (7)        // Camera communication
#define I2C2_IRQ_PRIORITY                   (7)        // Display communication



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
        GPIOA->AFR[0] &= ~(0x0Fu << (pin * 4u)); // Clear
        GPIOA->AFR[0] |=  (af    << (pin * 4u)); // Set
    } else {
        GPIOA->AFR[1] &= ~(0x0Fu << (pin * 4u - 32u)); // Clear
        GPIOA->AFR[1] |=  (af    << (pin * 4u - 32u)); // Set
    }
}

#endif // _PROJECT_BASE_H_
