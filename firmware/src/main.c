#include "stm32f373xc.h"
#include "project_base.h"
#include <stdbool.h>


static void system_setup(void);
static void systimer_setup(void);
static void setup_debug_gpio(void);


int main()
{
    system_setup();
    systimer_setup();
    setup_debug_gpio();
    
    while (1);
}

static void system_setup(void) {
    
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
    
    // Switch USARTx and I2Cx clock source to system clock
    RCC->CFGR3 |= RCC_CFGR3_I2C1SW_SYSCLK | RCC_CFGR3_I2C2SW_SYSCLK | 
                  RCC_CFGR3_USART2SW_SYSCLK | RCC_CFGR3_USART3SW_SYSCLK;
    
    // Enable GPIO clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN | 
                   RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOEEN | RCC_AHBENR_GPIOFEN;
}

static void systimer_setup(void) {
    
    // Systimer setup 
    SysTick->VAL = 0;
    SysTick->LOAD = SYSTEM_CLOCK_FREQUENCY / 1000;
    SysTick->CTRL = (1 << SysTick_CTRL_TICKINT_Pos) | (1 << SysTick_CTRL_CLKSOURCE_Pos) | (1 << SysTick_CTRL_ENABLE_Pos);
    
    // Enable systimer
    SysTick->CTRL |= (1 << SysTick_CTRL_ENABLE_Pos);
    NVIC_EnableIRQ(SysTick_IRQn);
}

static void setup_debug_gpio(void) {
    
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

void SysTick_Handler(void) {
  
    //GPIOA->ODR ^= (1 << DEBUG_PIN);
  
    DEBUG_TP1_PIN_TOGGLE;
    DEBUG_TP2_PIN_TOGGLE;
    DEBUG_TP3_PIN_TOGGLE;
    DEBUG_TP4_PIN_TOGGLE;
    asm("NOP");
}

void HardFault_Handler(void) {
    
    while (true);
}