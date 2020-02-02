#include "stm32f373xc.h"
#include "project_base.h"
#include "pwm.h"
#include "systimer.h"
#include "usart2.h"
#include "i2c1.h"
#include "adc.h"
#include "communication.h"
#include "system_monitor.h"
#include <stdbool.h>
#include <stdio.h>


static void system_init(void);
static void debug_gpio_init(void);


uint8_t rx_buffer[1024] = {0};
uint8_t tx_buffer[1024] = {0};



void main() {
    
    system_init();
    systimer_init();
    debug_gpio_init();

    sysmon_init();
    communication_init();
    
    while (true) {
        
        sysmon_process();
        communication_process();
    }
}

static void system_init(void) {
    
    // Enable HSE
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) != RCC_CR_HSERDY);
    
    // Configre and enable PLL
    RCC->CFGR |= RCC_CFGR_PLLSRC | (0x07 << RCC_CFGR_PLLMUL_Pos);
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY);
    
    // Setup deviders for AHB(1), APB1(2), APB2(1)
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV1;
    
    // Change FLASH latency
    FLASH->ACR |= (0x02 << FLASH_ACR_LATENCY_Pos);
    
    // Switch system clocks to PLL
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS_PLL) != RCC_CFGR_SWS_PLL);
    
    // Switch USARTx clock source to system clock
    RCC->CFGR3 |= RCC_CFGR3_USART2SW_SYSCLK | RCC_CFGR3_USART3SW_SYSCLK;
    
    
    
    // Enable GPIO clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN | 
                   RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOEEN | RCC_AHBENR_GPIOFEN;
    
    // Enable clocks for DMA1
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    while ((RCC->AHBENR & RCC_AHBENR_DMA1EN) == 0);

    // Enable clocks for TIM17
    RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
    while ((RCC->APB2ENR & RCC_APB2ENR_TIM17EN) == 0);    

    // Enable clocks for USART2
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    while ((RCC->APB1ENR & RCC_APB1ENR_USART2EN) == 0);
    
    // Enable clocks for I2C1
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    while ((RCC->APB1ENR & RCC_APB1ENR_I2C1EN) == 0);
    
    // Enable clocks for ADC1
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    while ((RCC->APB2ENR & RCC_APB2ENR_ADC1EN) == 0);
}

static void debug_gpio_init(void) {
    
    // TP1 pin (PC9): output mode, push-pull, high speed, no pull
    GPIOC->BRR      =  (0x01 << (DEBUG_TP1_PIN * 1));
    GPIOC->MODER   |=  (0x01 << (DEBUG_TP1_PIN * 2));
    GPIOC->OSPEEDR |=  (0x03 << (DEBUG_TP1_PIN * 2));
    GPIOC->PUPDR   &= ~(0x03 << (DEBUG_TP1_PIN * 2));
    
    // TP2 pin (PA8): output mode, push-pull, high speed, no pull
    GPIOA->BRR      =  (0x01 << (DEBUG_TP2_PIN * 1));
    GPIOA->MODER   |=  (0x01 << (DEBUG_TP2_PIN * 2));
    GPIOA->OSPEEDR |=  (0x03 << (DEBUG_TP2_PIN * 2));
    GPIOA->PUPDR   &= ~(0x03 << (DEBUG_TP2_PIN * 2));
    
    // TP3 pin (PC12): output mode, push-pull, high speed, no pull
    GPIOC->BRR      =  (0x01 << (DEBUG_TP3_PIN * 1));
    GPIOC->MODER   |=  (0x01 << (DEBUG_TP3_PIN * 2));
    GPIOC->OSPEEDR |=  (0x03 << (DEBUG_TP3_PIN * 2));
    GPIOC->PUPDR   &= ~(0x03 << (DEBUG_TP3_PIN * 2));
    
    // TP4 pin (PD2): output mode, push-pull, high speed, no pull
    GPIOD->BRR      =  (0x01 << (DEBUG_TP4_PIN * 1));
    GPIOD->MODER   |=  (0x01 << (DEBUG_TP4_PIN * 2));
    GPIOD->OSPEEDR |=  (0x03 << (DEBUG_TP4_PIN * 2));
    GPIOD->PUPDR   &= ~(0x03 << (DEBUG_TP4_PIN * 2));
}

void HardFault_Handler(void) {
    
    while (true);
}






