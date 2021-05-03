//  ***************************************************************************
/// @file    main.c
/// @author  NeoProg
//  ***************************************************************************
#include "project_base.h"
#include "systimer.h"
#include "hx711.h"
#include "i2c1.h"
#include "usart1.h"

static void system_init(void);


void EXTI_IRQHandler(void) {
    uint32_t pr = EXTI->PR;
    EXTI->PR = 0xFFFFFFFF;
    hx711_process_irq(pr);
}

/*


EXTI0_1_IRQn
EXTI2_3_IRQn
EXTI4_15_IRQn
*/

void frame_error_callback(void) {
    asm("NOP");
}

void frame_transmitted_callback(void) {
    asm("NOP");
}


//  ***************************************************************************
/// @brief  Program entry point
/// @param  none
/// @return none
//  ***************************************************************************
int main() {
    system_init();
    systimer_init();
    delay_ms(1000);
    
    usart1_callbacks_t callbacks;
    callbacks.frame_error_callback = frame_error_callback;
    callbacks.frame_transmitted_callback = frame_transmitted_callback;
    usart1_init(500000, &callbacks);
    
    int counter = 1;
    while (true) {
        uint8_t* tx_buffer = usart1_get_tx_buffer();
        tx_buffer[0] = 0xAA;
        tx_buffer[1] = counter;
        tx_buffer[2] = counter;
        tx_buffer[3] = counter;
        tx_buffer[4] = counter;
        tx_buffer[5] = counter;
        tx_buffer[6] = counter;
        tx_buffer[7] = counter;
        tx_buffer[8] = counter;
        tx_buffer[9] = counter;
        tx_buffer[10] = counter;
        tx_buffer[11] = counter;
        tx_buffer[12] = counter;
        
        tx_buffer[13] = counter;
        tx_buffer[14] = counter;
        tx_buffer[15] = counter;
        tx_buffer[16] = counter;
        tx_buffer[17] = counter;
        tx_buffer[18] = counter;
        tx_buffer[19] = 0xAA;
        counter++;
        usart1_start_tx(20);
        delay_ms(50);
    }
    
/*
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
    
    /*i2c1_init(I2C_SPEED_400KHZ);
    uint8_t buffer[10] = {0};
    static bool result = false;
    result = i2c1_read(0x68 << 1, 0x00, 1, buffer, 1);
    
    while (true) {}*/
    
    
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
    while ((RCC->CFGR3 & RCC_CFGR3_USART1SW_0) == 0);

    // Enable GPIO clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    while ((RCC->AHBENR & RCC_AHBENR_GPIOAEN) == 0);
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    while ((RCC->AHBENR & RCC_AHBENR_GPIOBEN) == 0);
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    while ((RCC->AHBENR & RCC_AHBENR_GPIOCEN) == 0);
    RCC->AHBENR |= RCC_AHBENR_GPIOFEN;
    while ((RCC->AHBENR & RCC_AHBENR_GPIOFEN) == 0);
    
    // Enable DMA
    RCC->AHBENR |= RCC_AHBENR_DMAEN;
    while ((RCC->AHBENR & RCC_AHBENR_DMAEN) == 0);
    
    // Enable timers
    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
    while ((RCC->APB1ENR & RCC_APB1ENR_TIM14EN) == 0);
    
    // Enable SYSCFG
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
    while ((RCC->APB2ENR & RCC_APB2ENR_SYSCFGCOMPEN) == 0);
    
    // Enable clocks for I2C1
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    while ((RCC->APB1ENR & RCC_APB1ENR_I2C1EN) == 0);
    
    // Enable USART1 clocks
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    while ((RCC->APB2ENR & RCC_APB2ENR_USART1EN) == 0);
}
