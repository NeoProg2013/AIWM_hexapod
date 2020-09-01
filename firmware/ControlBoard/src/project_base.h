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


#endif // _PROJECT_BASE_H_
