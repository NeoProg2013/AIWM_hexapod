//  ***************************************************************************
/// @file    main.c
/// @author  NeoProg
//  ***************************************************************************
#include "project_base.h"
#include "systimer.h"
#include "hx711.h"

static void system_init(void);




//  ***************************************************************************
/// @brief  Program entry point
/// @param  none
/// @return none
//  ***************************************************************************
int main() {
    system_init();
    
    #define HX711_CLK_PIN           GPIOA, 0
    gpio_set             (HX711_CLK_PIN);
    gpio_set_mode        (HX711_CLK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (HX711_CLK_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(HX711_CLK_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (HX711_CLK_PIN, GPIO_PULL_NO);
    
    systimer_init();
    delay_ms(1000);
    hx711_init();
    hx711_power_up();
    hx711_calibration();
    
    int32_t data[6] = {0};
    int32_t prev_data[6] = {0};
    hx711_read(data);
        
    for (int i = 0; i < 8; ++i) {
        prev_data[i] = data[i];  
    }
    
    while (true) {
        hx711_process();
        if (hx711_read(data)) {
            for (int i = 0; i < 8; ++i) {
                if (abs(prev_data[i] - data[i]) > 5000) {
                    asm("NOP");
                    asm("NOP");
                    asm("NOP");
                }
                prev_data[i] = data[i];
                
            }
            
        }
    }
}

//  ***************************************************************************
/// @brief  System initialization
/// @param  none
/// @return none
//  ***************************************************************************
static void system_init(void) {
    
    // Enable Prefetch Buffer
    FLASH->ACR = FLASH_ACR_PRFTBE;
    
    // Switch USARTx clock source to system clock
    RCC->CFGR3 |= RCC_CFGR3_USART1SW_0;

    // Enable GPIO clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOFEN;
}
