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




/*


EXTI0_1_IRQn
EXTI2_3_IRQn
EXTI4_15_IRQn
*/

static bool is_frame_transmitted = true;
void frame_error_callback(void) {
    asm("NOP");
}

void frame_transmitted_callback(void) {
    is_frame_transmitted = true;
}


//  ***************************************************************************
/// @brief  Program entry point
/// @param  none
/// @return none
//  ***************************************************************************
int main() {
    system_init();
    systimer_init();
    
    usart1_callbacks_t callbacks;
    callbacks.frame_error_callback = frame_error_callback;
    callbacks.frame_transmitted_callback = frame_transmitted_callback;
    usart1_init(500000, &callbacks);
    
    delay_ms(1000);
    
    hx711_init();
    hx711_power_up();
    hx711_calibration();
    
    uint64_t send_data_time = get_time_ms();
    bool hx711_is_first_read = true;
    //bool mpu6050_is_first_read = true;
    int16_t hx711_flt_data[6] = {0};
    int16_t mpu6050_flt_data[3] = {0};
    while (true) {
        
        // Read data from HX711
        bool hx711_process_result = hx711_process();
        if (hx711_is_data_ready()) { 
            int16_t raw_data[6] = {0};
            hx711_read(raw_data);
            for (uint32_t i = 0; i < 6; ++i) {
                if (hx711_is_first_read) {
                    hx711_flt_data[i] = raw_data[i];
                } else {
                    hx711_flt_data[i] = (int16_t)(raw_data[i] * 0.1f + hx711_flt_data[i] * (1 - 0.1f));
                }
            }
            hx711_is_first_read = false;
        }
        
        // Read data from MPU6050
        //
        //
        
        // Send data to MMCU
        if (get_time_ms() - send_data_time > 13 && is_frame_transmitted) {
            uint8_t* tx_buffer = usart1_get_tx_buffer();
            uint32_t tx_data_size = 0;
            
            // Write start frame marker
            tx_buffer[tx_data_size++] = 0xAA;
            
            // Write HX711 data
            for (uint32_t i = 0; i < 6; ++i) {
                if (!hx711_process_result) {
                    hx711_flt_data[i] = 0xFFFF;
                    hx711_is_first_read = true;
                }
                memcpy(&tx_buffer[tx_data_size], &hx711_flt_data[i], sizeof(hx711_flt_data[i]));
                tx_data_size += sizeof(hx711_flt_data[i]);
            }
            
            // Write MPU6050 data
            for (uint32_t i = 0; i < 3; ++i) {
                memcpy(&tx_buffer[tx_data_size], &mpu6050_flt_data[i], sizeof(mpu6050_flt_data[i]));
                tx_data_size += sizeof(mpu6050_flt_data[i]);
            }
            
            // Write end frame marker
            tx_buffer[tx_data_size++] = 0xAA;
            
            // Send frame
            is_frame_transmitted = false;
            usart1_start_tx(tx_data_size);
        }
    } 
}

//  ***************************************************************************
/// @brief  System initialization
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






//  ***************************************************************************
/// @brief  GPIO IRQ handler
//  ***************************************************************************
void EXTI_IRQHandler(void) {
    uint32_t pr = EXTI->PR;
    EXTI->PR = 0xFFFFFFFF;
    hx711_process_irq(pr);
}
