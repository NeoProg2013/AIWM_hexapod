//  ***************************************************************************
/// @file    usart2.c
/// @author  NeoProg
//  ***************************************************************************
#include "usart2.h"
#include "stm32f373xc.h"
#include "project_base.h"
#include <stdbool.h>

#define USART_TX_PIN                    (3) // PB3
#define USART_RX_PIN                    (4) // PB4


static void usart_reset(bool reset_tx, bool reset_rx);

static usart2_callbacks_t usart_callbacks;
static uint32_t last_buffer_size = 0;


//  ***************************************************************************
/// @brief  USART initialization
/// @param  callbacks: USART callbacks addresses
/// @param  baud_rate: USART baud rate
/// @return none
//  ***************************************************************************
void usart2_init(uint32_t baud_rate, usart2_callbacks_t* callbacks) {

    // Copy callbacks
    usart_callbacks = *callbacks;
    
    
    //
    // Setup GPIO
    //
    // Setup TX pin
    GPIOB->MODER   |=  (0x02u << (USART_TX_PIN * 2u)); // Alternate function mode
    GPIOB->OSPEEDR |=  (0x03u << (USART_TX_PIN * 2u)); // High speed
    GPIOB->PUPDR   &= ~(0x03u << (USART_TX_PIN * 2u)); // Disable pull
    GPIOB->AFR[0]  |=  (0x07u << (USART_TX_PIN * 4u)); // AF7
    
    // Setup RX pin
    GPIOB->MODER   |=  (0x02u << (USART_RX_PIN * 2u)); // Alternate function mode
    GPIOB->OSPEEDR |=  (0x03u << (USART_RX_PIN * 2u)); // High speed
    GPIOB->PUPDR   &= ~(0x03u << (USART_RX_PIN * 2u)); // Disable pull
    GPIOB->PUPDR   |=  (0x01u << (USART_RX_PIN * 2u)); // Enable pull up
    GPIOB->AFR[0]  |=  (0x07u << (USART_RX_PIN * 4u)); // AF7
    
    
    //
    // Setup USART2
    //
    RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;

    // Setup USART: 115200 8N2, DMA for TX and RX
    USART2->CR1  = USART_CR1_RTOIE;
    USART2->CR2  = USART_CR2_RTOEN | USART_CR2_STOP_1;
    USART2->CR3  = USART_CR3_DMAT | USART_CR3_DMAR | USART_CR3_EIE;
    USART2->BRR  = SYSTEM_CLOCK_FREQUENCY / baud_rate;
    USART2->RTOR = 35; // 3.5 char timer
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_SetPriority(USART2_IRQn, USART2_IRQ_PRIORITY);

    // Setup DMA channel for TX
    DMA1_Channel7->CCR  &= ~DMA_CCR_EN;
    DMA1_Channel7->CCR   = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TEIE | DMA_CCR_TCIE;
    DMA1_Channel7->CPAR  = (uint32_t)(&USART2->TDR);
    DMA1_Channel7->CMAR  = 0;
    DMA1_Channel7->CNDTR = 0;
    NVIC_EnableIRQ(DMA1_Channel7_IRQn);
    NVIC_SetPriority(DMA1_Channel7_IRQn, USART2_IRQ_PRIORITY);

    // Setup DMA channel for RX
    DMA1_Channel6->CCR  &= ~DMA_CCR_EN;
    DMA1_Channel6->CCR   = DMA_CCR_MINC | DMA_CCR_TEIE | DMA_CCR_TCIE;
    DMA1_Channel6->CPAR  = (uint32_t)(&USART2->RDR);
    DMA1_Channel6->CMAR  = 0;
    DMA1_Channel6->CNDTR = 0;
    NVIC_EnableIRQ(DMA1_Channel6_IRQn);
    NVIC_SetPriority(DMA1_Channel6_IRQn, USART2_IRQ_PRIORITY);

    // Enable USART
    USART2->CR1 |= USART_CR1_UE;
    usart_reset(true, true);
}

//  ***************************************************************************
/// @brief  USART set new baud rate
/// @param  baud_rate:new baud rate
/// @return none
//  ***************************************************************************
void usart2_set_baud_rate(uint32_t baud_rate) {

    // Disable DMA and USART
    usart_reset(true, true);
    USART2->CR1 &= ~USART_CR1_UE;

    // Change baud rate
    USART2->BRR  = SYSTEM_CLOCK_FREQUENCY / baud_rate;

    // Enable USART
    USART2->CR1 |= USART_CR1_UE;
}

//  ***************************************************************************
/// @brief  USART start frame TX
/// @param  tx_buffer: pointer to buffer
/// @param  bytes_count: bytes count for transmit
/// @return none
//  ***************************************************************************
void usart2_start_tx(uint8_t* tx_buffer, uint32_t bytes_count) {

    usart_reset(true, false);

    DMA1_Channel7->CMAR  = (uint32_t)tx_buffer;
    DMA1_Channel7->CNDTR = bytes_count;
    DMA1_Channel7->CCR  |= DMA_CCR_EN;
    USART2->CR1 |= USART_CR1_TE;
}

//  ***************************************************************************
/// @brief  USART start frame TX
/// @param  tx_buffer: pointer to buffer
/// @param  bytes_count: bytes count for receive
/// @return none
//  ***************************************************************************
void usart2_start_rx(uint8_t* rx_buffer, uint32_t buffer_size) {

    usart_reset(false, true);

    DMA1_Channel6->CMAR  = (uint32_t)rx_buffer;
    DMA1_Channel6->CNDTR = buffer_size;
    DMA1_Channel6->CCR  |= DMA_CCR_EN;
    USART2->CR1 |= USART_CR1_RE;

    last_buffer_size = buffer_size;
}





//  ***************************************************************************
/// @brief  USART reset transmitted or\and receiver
/// @note   Clear interrupt flags according interrupt mapping diagram (figure 222 of Reference Manual)
/// @param  reset_tx: true - reset transmitter
/// @param  reset_rx: true - reset receiver
/// @return none
//  ***************************************************************************
static void usart_reset(bool reset_tx, bool reset_rx) {

    if (reset_tx == true) {
        USART2->CR1 &= ~USART_CR1_TE;
        USART2->ICR |= USART_ICR_FECF;
        DMA1_Channel7->CCR &= ~DMA_CCR_EN;
        DMA1->IFCR = DMA_IFCR_CGIF7;
    }

    if (reset_rx == true) {
        USART2->CR1 &= ~USART_CR1_RE;
        USART2->ICR |= USART_ICR_RTOCF | USART_ICR_FECF | USART_ICR_NCF | USART_ICR_ORECF | USART_ICR_PECF;
        DMA1_Channel6->CCR &= ~DMA_CCR_EN;
        DMA1->IFCR = DMA_IFCR_CGIF6;
    }
}





//  ***************************************************************************
/// @brief  DMA channel ISR for transmitter
/// @param  none
/// @return none
//  ***************************************************************************
void DMA1_Channel7_IRQHandler(void) {

    uint32_t status = DMA1->ISR;

    if (status & DMA_ISR_TCIF7) {   // Frame transmit complete
        usart_reset(true, false);
        usart_callbacks.frame_transmitted_callback();
    }
    if (status & DMA_ISR_TEIF7) {   // DMA memory access error
        usart_reset(true, false);
        usart_callbacks.error_callback();
    }
}

//  ***************************************************************************
/// @brief  DMA channel ISR for receiver
/// @param  none
/// @return none
//  ***************************************************************************
void DMA1_Channel6_IRQHandler(void) {

    uint32_t status = DMA1->ISR;

    if (status & DMA_ISR_TCIF6) {
        usart_reset(false, true);
        usart_callbacks.error_callback();
    }
    if (status & DMA_ISR_TEIF6) {   // DMA memory access error
        usart_reset(false, true);
        usart_callbacks.error_callback();
    }
}

//  ***************************************************************************
/// @brief  USART ISR
/// @param  none
/// @return none
//  ***************************************************************************
void USART2_IRQHandler(void) {

    uint32_t status = USART2->ISR;

    if (status & USART_ISR_RTOF) {
        usart_reset(false, true);
        usart_callbacks.frame_received_callback(last_buffer_size - DMA1_Channel6->CNDTR);
    }

    if (status & (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE | USART_ISR_PE)) {
        usart_reset(true, true);
        usart_callbacks.error_callback();
    }
}
