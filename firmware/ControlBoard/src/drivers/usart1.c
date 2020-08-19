//  ***************************************************************************
/// @file    usart1.c
/// @author  NeoProg
//  ***************************************************************************
#include "usart1.h"
#include "stm32f373xc.h"
#include "project_base.h"
#include <stdbool.h>

#define USART_TX_PIN                    (3) // PA9
#define USART_RX_PIN                    (4) // PA10


static void usart_reset(void);


//  ***************************************************************************
/// @brief  USART initialization
/// @param  baud_rate: USART baud rate
/// @return none
//  ***************************************************************************
void usart1_init(uint32_t baud_rate) {

    //
    // Setup GPIO
    //
    // Setup TX pin
    GPIOA->MODER   |=  (0x02u << (USART_TX_PIN * 2u)); // Alternate function mode
    GPIOA->OSPEEDR |=  (0x03u << (USART_TX_PIN * 2u)); // High speed
    GPIOA->PUPDR   &= ~(0x03u << (USART_TX_PIN * 2u)); // Disable pull
    GPIOA->AFR[1]  |=  (0x07u << ((USART_TX_PIN * 4u) - 32)); // AF7
    
    // Setup RX pin
    GPIOA->MODER   |=  (0x02u << (USART_RX_PIN * 2u)); // Alternate function mode
    GPIOA->OSPEEDR |=  (0x03u << (USART_RX_PIN * 2u)); // High speed
    GPIOA->PUPDR   &= ~(0x03u << (USART_RX_PIN * 2u)); // Disable pull
    GPIOA->PUPDR   |=  (0x01u << (USART_RX_PIN * 2u)); // Enable pull up
    GPIOA->AFR[1]  |=  (0x07u << ((USART_RX_PIN * 4u) - 32)); // AF7
    
    
    //
    // Setup USART
    //
    RCC->APB1RSTR |= RCC_APB1RSTR_USART1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USART1RST;

    // Setup USART: 115200 8N2, DMA for TX and RX
    USART1->CR2  = USART_CR2_STOP_1;
    USART1->BRR  = SYSTEM_CLOCK_FREQUENCY / baud_rate;
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, USART1_IRQ_PRIORITY);

    // Enable USART
    USART1->CR1 |= USART_CR1_UE;
    usart_reset();
}

bool usart1_read(uint8_t* buffer, uint32_t count) {
    
}

bool usart1_write(uint8_t* buffer, uint32_t count) {
    
    // Enable TX
    USART1->CR1 |= USART_CR1_TE;
    
    // Send data loop
    bool result = false;
    while (count) {
        
        // Wait TXE status
        uint64_t start_time = get_time_ms();
        while ((USART1->ISR & USART_ISR_RXNE) == 0) {
            if (get_time_ms() - start_time > 100) {
                goto usart1_write_end;
            }
        }
        
        // Send byte
        
    }
    result = true;
    
usart1_write_end:
    USART1->CR1 &= ~USART_CR1_TE;
    USART1->ICR |= USART_ICR_FECF;
    return result;
}





//  ***************************************************************************
/// @brief  USART reset transmitted or\and receiver
/// @note   Clear interrupt flags according interrupt mapping diagram (figure 222 of Reference Manual)
/// @return none
//  ***************************************************************************
static void usart_reset(void) {

    // Reset TX
    USART1->CR1 &= ~USART_CR1_TE;
    USART1->ICR |= USART_ICR_FECF;
        
    // Reset RX
    USART1->CR1 &= ~USART_CR1_RE;
    USART1->ICR |= USART_ICR_RTOCF | USART_ICR_FECF | USART_ICR_NCF | USART_ICR_ORECF | USART_ICR_PECF;
}





//  ***************************************************************************
/// @brief  USART ISR
/// @param  none
/// @return none
//  ***************************************************************************
void USART2_IRQHandler(void) {

    uint32_t status = USART1->ISR;

    if (status & (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE | USART_ISR_PE)) {
        usart_reset(true, true);
    }
    if (status & USART_ISR_RXNE) {
        
    }
    if (status & USART_ISR_TXE) {
        
    }
}
