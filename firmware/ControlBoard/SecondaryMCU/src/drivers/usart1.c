//  ***************************************************************************
/// @file    usart1.c
/// @author  NeoProg
//  ***************************************************************************
#include "usart1.h"
#include "project_base.h"

#define USART_TX_PIN                    (2)  // PA2


static uint8_t tx_buffer[64] = {0};
static usart1_callbacks_t usart_callbacks;


static void usart_reset();


//  ***************************************************************************
/// @brief  USART initialization
/// @param  baud_rate: USART baud rate
//  ***************************************************************************
void usart1_init(uint32_t baud_rate, usart1_callbacks_t* callbacks) {
    usart_callbacks = *callbacks;

    // Setup TX pin
    gpio_set_mode        (GPIOA, USART_TX_PIN, GPIO_MODE_AF);
    gpio_set_output_speed(GPIOA, USART_TX_PIN, GPIO_SPEED_HIGH);
    gpio_set_pull        (GPIOA, USART_TX_PIN, GPIO_PULL_NO);
    gpio_set_af          (GPIOA, USART_TX_PIN, 1);

    // Setup USART: 8N1, DMA for TX
    RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
    USART1->CR3 = USART_CR3_DMAT | USART_CR3_EIE;
    USART1->BRR = SYSTEM_CLOCK_FREQUENCY / baud_rate;
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, USART1_IRQ_PRIORITY);
    
    // Setup DMA channel for TX
    DMA1_Channel2->CCR  &= ~DMA_CCR_EN;
    DMA1_Channel2->CCR   = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TEIE | DMA_CCR_TCIE;
    DMA1_Channel2->CPAR  = (uint32_t)(&USART1->TDR);
    DMA1_Channel2->CMAR  = 0;
    DMA1_Channel2->CNDTR = 0;
    NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
    NVIC_SetPriority(DMA1_Channel2_3_IRQn, USART1_IRQ_PRIORITY);

    // Enable USART
    USART1->CR1 |= USART_CR1_UE;
    usart_reset();
}

//  ***************************************************************************
/// @brief  USART start frame transmit
/// @param  bytes_count: bytes count for transmit
//  ***************************************************************************
void usart1_start_tx(uint32_t bytes_count) {
    if (bytes_count) {  
        usart_reset();
        DMA1_Channel2->CMAR  = (uint32_t)tx_buffer;
        DMA1_Channel2->CNDTR = bytes_count;
        DMA1_Channel2->CCR  |= DMA_CCR_EN;
        USART1->CR1 |= USART_CR1_TE;
    }
}

//  ***************************************************************************
/// @brief  Get USART TX buffer address
/// @return TX buffer address
//  ***************************************************************************
uint8_t* usart1_get_tx_buffer(void) {
    return tx_buffer;
}





//  ***************************************************************************
/// @brief  USART reset transmitted or\and receiver
/// @note   Clear interrupt flags according interrupt mapping diagram (figure 222 of Reference Manual)
/// @param  reset_tx: true - reset TX
/// @param  reset_rx: true - reset RX
//  ***************************************************************************
static void usart_reset() {
    USART1->CR1 &= ~USART_CR1_TE; // Disable TX
    USART1->ICR |= USART_ICR_FECF | USART_ICR_TCCF;
    DMA1_Channel2->CCR &= ~DMA_CCR_EN;
    DMA1->IFCR = DMA_IFCR_CGIF2;
}





//  ***************************************************************************
/// @brief  DMA channel ISR for transmitter
//  ***************************************************************************
void DMA1_Channel2_3_IRQHandler(void) {
    uint32_t status = DMA1->ISR;
    if (status & DMA_ISR_TCIF2) { // Frame transmit complete
        usart_reset();
        usart_callbacks.frame_transmitted_callback();
    }
    if (status & DMA_ISR_TEIF2) { // DMA memory access error
        usart_reset();
        usart_callbacks.frame_error_callback();
    }
}

//  ***************************************************************************
/// @brief  USART ISR
//  ***************************************************************************
void USART1_IRQHandler(void) {
    uint32_t status = USART1->ISR;
    if (status & (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE | USART_ISR_PE)) {
        usart_reset();
        usart_callbacks.frame_error_callback();
        return;
    }
}
