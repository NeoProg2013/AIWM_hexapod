//  ***************************************************************************
/// @file    indication.c
/// @author  NeoProg
//  ***************************************************************************
#include "indication.h"
#include "stm32f373xc.h"
#include "system_monitor.h"
#include "systimer.h"
#include <stdint.h>
#include <stdbool.h>

#define LED_R_PIN                   (5) // PB5
#define LED_G_PIN                   (6) // PB6
#define LED_B_PIN                   (7) // PB7
#define BUZZER_PIN                  (0) // PC0

#define LED_TURN_OFF(pin)           ( GPIOB->BRR  = (uint32_t)(0x01 << ((pin) * 1)) )
#define LED_TURN_ON(pin)            ( GPIOB->BSRR = (uint32_t)(0x01 << ((pin) * 1)) )
#define BUZZER_TURN_OFF()           ( GPIOC->BRR  = (uint32_t)(0x01 << (BUZZER_PIN * 1)) )
#define BUZZER_TURN_ON()            ( GPIOC->BSRR = (uint32_t)(0x01 << (BUZZER_PIN * 1)) )


static void blink_red_led_with_buzzer(uint32_t period);
static void blink_yellow_led(uint32_t period);


//  ***************************************************************************
/// @brief  Driver initialization
/// @param  none
/// @return none
//  ***************************************************************************
void indication_init(void) {

    // Front LED red pin (PB5): output mode, push-pull, high speed, no pull
    GPIOB->BRR      =  (0x01 << (LED_R_PIN * 1));
    GPIOB->MODER   |=  (0x01 << (LED_R_PIN * 2));
    GPIOB->OSPEEDR |=  (0x03 << (LED_R_PIN * 2));
    GPIOB->PUPDR   &= ~(0x03 << (LED_R_PIN * 2));
    
    // Front LED green pin (PB6): output mode, push-pull, high speed, no pull
    GPIOB->BRR      =  (0x01 << (LED_G_PIN * 1));
    GPIOB->MODER   |=  (0x01 << (LED_G_PIN * 2));
    GPIOB->OSPEEDR |=  (0x03 << (LED_G_PIN * 2));
    GPIOB->PUPDR   &= ~(0x03 << (LED_G_PIN * 2));
    
    // Front LED blue pin (PB7): output mode, push-pull, high speed, no pull
    GPIOB->BRR      =  (0x01 << (LED_B_PIN * 1));
    GPIOB->MODER   |=  (0x01 << (LED_B_PIN * 2));
    GPIOB->OSPEEDR |=  (0x03 << (LED_B_PIN * 2));
    GPIOB->PUPDR   &= ~(0x03 << (LED_B_PIN * 2));
    
    // Buzzer pin (PC0): output mode, push-pull, high speed, no pull
    GPIOC->BRR      =  (0x01 << (0 * 1));
    GPIOC->MODER   |=  (0x01 << (0 * 2));
    GPIOC->OSPEEDR |=  (0x03 << (0 * 2));
    GPIOC->PUPDR   &= ~(0x03 << (0 * 2));
    
    // Disable all
    LED_TURN_OFF(LED_R_PIN);
    LED_TURN_OFF(LED_G_PIN);
    LED_TURN_OFF(LED_B_PIN);
    BUZZER_TURN_OFF();
    
    // Blink R led
    LED_TURN_ON(LED_R_PIN);
    BUZZER_TURN_ON();
    delay_ms(100);
    LED_TURN_OFF(LED_R_PIN);
    BUZZER_TURN_OFF();
    delay_ms(100);
    
    // Blink G led
    LED_TURN_ON(LED_G_PIN);
    BUZZER_TURN_ON();
    delay_ms(100);
    LED_TURN_OFF(LED_G_PIN);
    BUZZER_TURN_OFF();
    delay_ms(100);
    
    // Blink B led
    LED_TURN_ON(LED_B_PIN);
    BUZZER_TURN_ON();
    delay_ms(100);
    LED_TURN_OFF(LED_B_PIN);
    BUZZER_TURN_OFF();
    delay_ms(100);
    
    // Blink RGB leds
    for (uint32_t i = 0; i < 3; ++i) {
        LED_TURN_ON(LED_R_PIN);
        LED_TURN_ON(LED_G_PIN);
        LED_TURN_ON(LED_B_PIN);
        BUZZER_TURN_ON();
        delay_ms(100);
        LED_TURN_OFF(LED_R_PIN);
        LED_TURN_OFF(LED_G_PIN);
        LED_TURN_OFF(LED_B_PIN);
        BUZZER_TURN_OFF();
        delay_ms(100);
    }
    
}

//  ***************************************************************************
/// @brief  Driver process
/// @param  none
/// @return none
//  ***************************************************************************
void indication_process(void) {
    
    if (sysmon_is_error_set(SYSMON_ANY_ERROR) == false) {
        LED_TURN_OFF(LED_R_PIN);
        LED_TURN_ON(LED_G_PIN);
        LED_TURN_OFF(LED_B_PIN);
        BUZZER_TURN_OFF();
    }
    else {
        
        if (sysmon_is_error_set(SYSMON_FATAL_ERROR) == true) {
             blink_red_led_with_buzzer(200);
        } 
        else if (sysmon_is_error_set(SYSMON_VOLTAGE_ERROR) == true) {
             blink_red_led_with_buzzer(500);
        }
        else if (sysmon_is_error_set(SYSMON_CONN_LOST_ERROR) == true) {
             blink_yellow_led(500);
        }
        else {
            LED_TURN_ON(LED_R_PIN);
            LED_TURN_ON(LED_G_PIN);
            LED_TURN_OFF(LED_B_PIN);
            BUZZER_TURN_OFF();
        }
    }
}




static void blink_red_led_with_buzzer(uint32_t period) {
    
    static uint32_t start_time = 0;
    static bool state = false;
    
    if (get_time_ms() - start_time > period) {
        
        if (state == false) {
            LED_TURN_ON(LED_R_PIN);
            LED_TURN_OFF(LED_G_PIN);
            LED_TURN_OFF(LED_B_PIN);
            BUZZER_TURN_ON();
        }
        else {
            LED_TURN_OFF(LED_R_PIN);
            LED_TURN_OFF(LED_G_PIN);
            LED_TURN_OFF(LED_B_PIN);
            BUZZER_TURN_OFF();
        }
        state = !state;
        start_time = get_time_ms();
    }
}

static void blink_yellow_led(uint32_t period) {
    
    static uint32_t start_time = 0;
    static bool state = false;
    
    if (get_time_ms() - start_time > period) {
        
        if (state == false) {
            LED_TURN_ON(LED_R_PIN);
            LED_TURN_ON(LED_G_PIN);
            LED_TURN_OFF(LED_B_PIN);
            BUZZER_TURN_OFF();
        }
        else {
            LED_TURN_OFF(LED_R_PIN);
            LED_TURN_OFF(LED_G_PIN);
            LED_TURN_OFF(LED_B_PIN);
            BUZZER_TURN_OFF();
        }
        state = !state;
        start_time = get_time_ms();
    }
}
