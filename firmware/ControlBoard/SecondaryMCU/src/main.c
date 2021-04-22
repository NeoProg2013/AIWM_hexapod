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
    systimer_init();
    
    gpio_set_mode(GPIOA, 4, GPIO_MODE_AF);
    gpio_set_output_speed(GPIOA, 4, GPIO_SPEED_HIGH);
    gpio_set_af(GPIOA, 4, 4);
    
    TIM14->PSC = 0x0000;
    TIM14->CCR1 = 14;
    TIM14->ARR  = TIM14->CCR1 * 2;
    TIM14->CCMR1 |= (0x06 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1FE;
    TIM14->CCER |= TIM_CCER_CC1E;
    TIM14->CR1 |= TIM_CR1_CEN;
    
    while (true) {
        
        
    }
    
    /*#define HX711_CLK_PIN           GPIOA, 0
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
    }*/
}

//  ***************************************************************************
/// @brief  System initialization
/// @param  none
/// @return none
//  ***************************************************************************
static void system_init(void) {
    
    // Enable Prefetch Buffer
    FLASH->ACR = FLASH_ACR_PRFTBE;
    
    // Configure PLL (clock source HSI/2 = 4MHz)
    RCC->CFGR |= RCC_CFGR_PLLMULL12;
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0);
    
    // Set FLASH latency
    FLASH->ACR |= FLASH_ACR_LATENCY;
    
    // Switch system clock to PLL
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS_PLL) == 0);
    
    // Switch USARTx clock source to system clock
    RCC->CFGR3 |= RCC_CFGR3_USART1SW_0;

    // Enable GPIO clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOFEN;
    
    // Enable timers
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
}
