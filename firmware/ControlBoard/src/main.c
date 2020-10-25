//  ***************************************************************************
/// @file    main.c
/// @author  NeoProg
//  ***************************************************************************
#include "project_base.h"
#include "configurator.h"
#include "system_monitor.h"
#include "swlp.h"
#include "cli.h"
#include "servo_driver.h"
#include "motion_core.h"
#include "sequences_engine.h"
#include "indication.h"
#include "gui.h"
#include "camera.h"
#include "pwm.h"
#include "systimer.h"


static void system_init(void);
static void debug_gpio_init(void);
static void emergency_loop(void);



//  ***************************************************************************
/// @brief  Program entry point
/// @param  none
/// @return none
//  ***************************************************************************
void main() {
    
    // System initialization
    system_init();
    systimer_init();
    debug_gpio_init();
    
    // Base module initialation
    sysmon_init();
    swlp_init();
    cli_init();
    indication_init();
    gui_init();
    
    // Initialation and check EEPROM intergity
    config_init();
    if (config_check_intergity() == false) {
        emergency_loop();
    }
    
    // Initializaion submodules
    camera_init();    
    sequences_engine_init();
    
    delay_ms(100);
    while (true) {
        
        sysmon_process();
        swlp_process();
        cli_process();
        indication_process();
        gui_process();
        
        camera_process();
        
        // Override select sequence if need
        if (sysmon_is_error_set(SYSMON_CONN_LOST_ERROR) == true) {
            sequences_engine_select_sequence(SEQUENCE_DOWN, 0, 0);
        }
        // Disable servo power if low supply voltage
        if (sysmon_is_error_set(SYSMON_VOLTAGE_ERROR) == true) {
            sequences_engine_select_sequence(SEQUENCE_DOWN, 0, 0);
            servo_driver_power_off();
        }
        
        // Motion process
        // This 3 functions should be call in this sequence
        sequences_engine_process();
        motion_core_process();
        servo_driver_process();
        
        // Check system failure
        if (sysmon_is_error_set(SYSMON_FATAL_ERROR) == true) {
            servo_driver_power_off();
            emergency_loop();
        }
    }
}

//  ***************************************************************************
/// @brief  Emergency loop
/// @param  none
/// @return none
//  ***************************************************************************
static void emergency_loop(void) {
    
    while (true) {
        sysmon_process();
        swlp_process();
        cli_process();
        indication_process();
        gui_process();
    }
}

//  ***************************************************************************
/// @brief  System initialization
/// @param  none
/// @return none
//  ***************************************************************************
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
    
    // Enable MCO
    RCC->CFGR |= RCC_CFGR_MCOSEL_SYSCLK;
    
    // Switch USARTx clock source to system clock
    RCC->CFGR3 |= RCC_CFGR3_USART3SW_SYSCLK | RCC_CFGR3_USART2SW_SYSCLK | RCC_CFGR3_USART1SW_SYSCLK;
    
    
    
    // Enable GPIO clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN | 
                   RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOEEN | RCC_AHBENR_GPIOFEN;
    
    // Enable clocks for DMA1
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    while ((RCC->AHBENR & RCC_AHBENR_DMA1EN) == 0);

    // Enable clocks for TIM17
    RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
    while ((RCC->APB2ENR & RCC_APB2ENR_TIM17EN) == 0);    
    
    // Enable clocks for USART3
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    while ((RCC->APB1ENR & RCC_APB1ENR_USART3EN) == 0);

    // Enable clocks for USART2
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    while ((RCC->APB1ENR & RCC_APB1ENR_USART2EN) == 0);
    
    // Enable clocks for USART1
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    while ((RCC->APB2ENR & RCC_APB2ENR_USART1EN) == 0);
    
    // Enable clocks for I2C1
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    while ((RCC->APB1ENR & RCC_APB1ENR_I2C1EN) == 0);
    
    // Enable clocks for I2C1
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    while ((RCC->APB1ENR & RCC_APB1ENR_I2C2EN) == 0);
    
    // Enable clocks for ADC1 (12Mhz max)
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    while ((RCC->APB2ENR & RCC_APB2ENR_ADC1EN) == 0);
}

static void debug_gpio_init(void) {
    
    // TP1 pin (PC9): output mode, push-pull, high speed, no pull
    GPIOC->BRR      =  (0x01u << (DEBUG_TP1_PIN * 1u));
    GPIOC->MODER   |=  (0x01u << (DEBUG_TP1_PIN * 2u));
    GPIOC->OSPEEDR |=  (0x03u << (DEBUG_TP1_PIN * 2u));
    GPIOC->PUPDR   &= ~(0x03u << (DEBUG_TP1_PIN * 2u));
    
    // TP3 pin (PA11): output mode, push-pull, high speed, no pull
    GPIOA->BRR      =  (0x01u << (DEBUG_TP3_PIN * 1));
    GPIOA->MODER   |=  (0x01u << (DEBUG_TP3_PIN * 2));
    GPIOA->OSPEEDR |=  (0x03u << (DEBUG_TP3_PIN * 2));
    GPIOA->PUPDR   &= ~(0x03u << (DEBUG_TP3_PIN * 2));
}

void HardFault_Handler(void) {
    while (true);
}
