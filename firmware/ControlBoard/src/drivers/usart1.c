//  ***************************************************************************
/// @file    usart1.c
/// @author  NeoProg
//  ***************************************************************************
#include "usart1.h"
#include "stm32f373xc.h"
#include "project_base.h"
#include "usart1_queue.h"
#include "systimer.h"
#include <stdbool.h>

#define USART_TX_PIN                    (9)  // PA9
#define USART_RX_PIN                    (10) // PA10

#define USART_TX_BYTE_TIMEOUT           (10) // ms


static void usart_reset(bool reset_tx, bool reset_rx);


//  ***************************************************************************
/// @brief  USART initialization
/// @param  baud_rate: USART baud rate
/// @return none
//  ***************************************************************************
void usart1_init(uint32_t baud_rate) {
    
    usart1_queue_init();

    //
    // Setup GPIO
    //
    // Setup TX pin
    GPIOA->MODER   |=  (0x02u << (USART_TX_PIN * 2u)); // Alternate function mode
    GPIOA->OSPEEDR |=  (0x03u << (USART_TX_PIN * 2u)); // High speed
    GPIOA->PUPDR   &= ~(0x03u << (USART_TX_PIN * 2u)); // Disable pull
    GPIOA->AFR[1]  |=  (0x07u << (USART_TX_PIN * 4u - 32)); // AF7
    
    // Setup RX pin
    GPIOA->MODER   |=  (0x02u << (USART_RX_PIN * 2u)); // Alternate function mode
    GPIOA->OSPEEDR |=  (0x03u << (USART_RX_PIN * 2u)); // High speed
    GPIOA->PUPDR   &= ~(0x03u << (USART_RX_PIN * 2u)); // Disable pull
    GPIOA->PUPDR   |=  (0x01u << (USART_RX_PIN * 2u)); // Enable pull up
    GPIOA->AFR[1]  |=  (0x07u << (USART_RX_PIN * 4u - 32)); // AF7
    
    
    //
    // Setup USART
    //
    RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;

    // Setup USART: 115200 8N2, DMA for TX and RX
    USART1->CR1  = USART_CR1_RXNEIE;
    USART1->CR2  = USART_CR2_STOP_1;
    USART1->CR3  = USART_CR3_EIE;
    USART1->BRR  = SYSTEM_CLOCK_FREQUENCY / baud_rate;
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, USART1_IRQ_PRIORITY);

    // Enable USART
    USART1->CR1 |= USART_CR1_UE;
    usart_reset(true, true);
}

//  ***************************************************************************
/// @brief  Check USART ready read
/// @param  none
/// @return true - data is available, false - no data
//  ***************************************************************************
bool usart1_is_ready_read(void) {
    return !usart1_queue_is_empty();
}

//  ***************************************************************************
/// @brief  Read data from USART queue
/// @param  buffer: pointer to buffer for data
/// @return true - read success, false - queue is empty
//  ***************************************************************************
bool usart1_read(uint8_t* buffer) {
    return usart1_queue_dequeue(buffer);
}

//  ***************************************************************************
/// @brief  Send data
/// @param  data: data for send
/// @param  count: bytes count for send
/// @return true - send success, false - error
//  ***************************************************************************
bool usart1_write(uint8_t* data, uint32_t count) {
    
    // Enable TX
    USART1->CR1 |= USART_CR1_TE;
    
    // Send data loop
    bool result = false;
    while (count) {
        
        // Wait TXE status
        uint64_t start_time = get_time_ms();
        while ((USART1->ISR & USART_ISR_TXE) == 0) {
            if (get_time_ms() - start_time > USART_TX_BYTE_TIMEOUT) {
                goto usart1_write_end;
            }
        }
        
        // Send byte
        USART1->TDR = (*data);
        ++data;
        --count;
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
/// @param  reset_tx: true - reset TX
/// @param  reset_rx: true - reset RX
/// @return none
//  ***************************************************************************
static void usart_reset(bool reset_tx, bool reset_rx) {

    // Reset TX
    if (reset_tx) {
        USART1->CR1 &= ~USART_CR1_TE;
        USART1->ICR |= USART_ICR_FECF;
    }
        
    // Reset RX
    if (reset_rx) {
        USART1->CR1 &= ~USART_CR1_RE;
        USART1->ICR |= USART_ICR_RTOCF | USART_ICR_FECF | USART_ICR_NCF | USART_ICR_ORECF | USART_ICR_PECF;
        USART1->CR1 |= USART_CR1_RE;
    }
}





//  ***************************************************************************
/// @brief  USART ISR
/// @param  none
/// @return none
//  ***************************************************************************
void USART1_IRQHandler(void) {

    uint32_t status = USART1->ISR;
    if (status & (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE | USART_ISR_PE)) {
        usart_reset(false, true);
        usart1_queue_clear();
    }
    if (status & USART_ISR_RXNE) {
        usart1_queue_enqueue(USART1->RDR);
    }
}
