/// ***************************************************************************
/// @file    main.c
/// @author  NeoProg
/// ***************************************************************************
#include "project-base.h"
#include "system-monitor.h"
#include "swlp.h"
#include "cli.h"
#include "servo-driver.h"
#include "motion-core.h"
#include "indication.h"
#include "display.h"
#include "pwm.h"
#include "pca9555.h"
#include "mpu6050.h"
#include "i2c1.h"
#include "i2c2.h"
#include "systimer.h"


static void system_init(void);
static void debug_gpio_init(void);
static void emergency_loop(void);


static bool is_ready = false;
static float data[2] = {0};



/// ***************************************************************************
/// @brief  Program entry point
/// @param  none
/// @return none
/// ***************************************************************************
void main() {
    // System initialization
    system_init();
    systimer_init();
    debug_gpio_init();
    i2c1_init(I2C1_SPEED_400KHZ);
    i2c2_init(I2C2_SPEED_400KHZ);
    
    //bool res = mpu6050_init();
    //mpu6050_set_state(true);
    //mpu6050_calibration();
    
    /*while (true) {
        
        if (mpu6050_is_data_ready()) {
            if (!mpu6050_read_data(data)) {
                asm("nop");
            }
        }
    }*/
    
    // Base module initialation
    sysmon_init();
    swlp_init();
    cli_init();
    indication_init();
    display_init();
    
    //pca9555_init();
    
    // Motion core nitializaion  
    servo_driver_init();
    motion_core_init();
    
    
    //pca9555_set_outputs(0xFFFF, PCA9555_GPIO_LED_RIGHT_3 | PCA9555_GPIO_LED_LEFT_1);
    
    //while (true) {
        /*delay_ms(120);
        pca9555_set_outputs(0xFFFF, PCA9555_GPIO_LED_RIGHT_1);
        delay_ms(120);
        pca9555_set_outputs(0xFFFF, PCA9555_GPIO_LED_RIGHT_2);
        delay_ms(120);
        pca9555_set_outputs(0xFFFF, PCA9555_GPIO_LED_RIGHT_3);
        
        delay_ms(120);
        pca9555_set_outputs(0xFFFF, PCA9555_GPIO_LED_LEFT_3);
        delay_ms(120);
        pca9555_set_outputs(0xFFFF, PCA9555_GPIO_LED_LEFT_2);
        delay_ms(120);
        pca9555_set_outputs(0xFFFF, PCA9555_GPIO_LED_LEFT_1);*/
    //}
    
    while (true) {
        
       /* if (pca9555_is_input_changed()) {
            asm("nop");
        }*/
        
        // Check system failure
        if (sysmon_is_error_set(SYSMON_FATAL_ERROR) == true) {
            servo_driver_power_off();
            emergency_loop();
        }
        
        // Override select sequence if need
        /*if (sysmon_is_error_set(SYSMON_CONN_LOST_ERROR) == true) {
            motion_core_select_script(MOTION_SCRIPT_DOWN);
        }*/
        // Disable servo power if low supply voltage
        /*if (sysmon_is_error_set(SYSMON_VOLTAGE_ERROR) == true) {
            motion_core_select_script(MOTION_SCRIPT_DOWN);
            servo_driver_power_off();
        }*/
        
        // Motion process
        // This 2 functions should be call in this sequence
        if (pwm_is_ready()) {
            pwm_set_lock_state(true);
            motion_core_process();
            servo_driver_process();
            pwm_set_lock_state(false);
        } 
        else { // Here is other operations
            sysmon_process();
            swlp_process();
            indication_process();
            display_process();
            cli_process();
        }
    }
}

/// ***************************************************************************
/// @brief  Emergency loop
/// ***************************************************************************
static void emergency_loop(void) {
    while (true) {
        sysmon_process();
        swlp_process();
        indication_process();
        display_process();
        cli_process();
    }
}

/// ***************************************************************************
/// @brief  System initialization
/// ***************************************************************************
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
    RCC->CFGR |= RCC_CFGR_MCOSEL_HSI;
    
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
    DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM17_STOP; // Stop PWM counter for debug mode
    
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
    
    // Enable clocks for I2C2
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    while ((RCC->APB1ENR & RCC_APB1ENR_I2C2EN) == 0);
    
    // Enable clocks for ADC1 (12Mhz max)
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    while ((RCC->APB2ENR & RCC_APB2ENR_ADC1EN) == 0);
}

static void debug_gpio_init(void) {
    // TP1 pin: output mode, push-pull, low speed, no pull
    gpio_reset           (DEBUG_TP1_PIN);
    gpio_set_mode        (DEBUG_TP1_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (DEBUG_TP1_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(DEBUG_TP1_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (DEBUG_TP1_PIN, GPIO_PULL_NO);
    
    // TP2 pin: MCO
    gpio_set_mode        (DEBUG_TP2_PIN, GPIO_MODE_AF);
    gpio_set_output_speed(DEBUG_TP2_PIN, GPIO_SPEED_HIGH);
    gpio_set_pull        (DEBUG_TP2_PIN, GPIO_PULL_NO);
    gpio_set_af          (DEBUG_TP2_PIN, 0);
    
    // TP3 pin: output mode, push-pull, low speed, no pull
    gpio_reset           (DEBUG_TP3_PIN);
    gpio_set_mode        (DEBUG_TP3_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (DEBUG_TP3_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(DEBUG_TP3_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (DEBUG_TP3_PIN, GPIO_PULL_NO);
    
    // TP4 pin: output mode, push-pull, low speed, no pull
    gpio_reset           (DEBUG_TP4_PIN);
    gpio_set_mode        (DEBUG_TP4_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (DEBUG_TP4_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(DEBUG_TP4_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (DEBUG_TP4_PIN, GPIO_PULL_NO);
    
    // TP5 pin: output mode, push-pull, low speed, no pull
    gpio_reset           (DEBUG_TP5_PIN);
    gpio_set_mode        (DEBUG_TP5_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (DEBUG_TP5_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(DEBUG_TP5_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (DEBUG_TP5_PIN, GPIO_PULL_NO);
}

#pragma call_graph_root="interrupt"
void HardFault_Handler(void) {
    while (true);
}
