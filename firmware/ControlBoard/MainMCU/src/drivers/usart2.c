//  ***************************************************************************
/// @file    usart2.c
/// @author  NeoProg
//  ***************************************************************************
#include "usart2.h"
#include "project-base.h"
#define USART_TX_PIN                    GPIOB, 3
#define USART_RX_PIN                    GPIOB, 4

static uint8_t tx_buffer[64] = {0};
static uint8_t rx_buffer[64] = {0};
static usart2_callbacks_t usart_callbacks;


static void usart_reset(bool reset_tx, bool reset_rx);


//  ***************************************************************************
/// @brief  USART initialization
/// @param  baud_rate: USART baud rate
/// @param  callbacks: USART callbacks
/// @return none
//  ***************************************************************************
void usart2_init(uint32_t baud_rate, usart2_callbacks_t* callbacks) {
    usart_callbacks = *callbacks;
    
    // Setup TX pin
    gpio_set_mode        (USART_TX_PIN, GPIO_MODE_AF);
    gpio_set_output_speed(USART_TX_PIN, GPIO_SPEED_HIGH);
    gpio_set_pull        (USART_TX_PIN, GPIO_PULL_NO);
    gpio_set_af          (USART_TX_PIN, 7);
    
    // Setup RX pin
    gpio_set_mode        (USART_RX_PIN, GPIO_MODE_AF);
    gpio_set_output_speed(USART_RX_PIN, GPIO_SPEED_HIGH);
    gpio_set_pull        (USART_RX_PIN, GPIO_PULL_UP);
    gpio_set_af          (USART_RX_PIN, 7);
    
    // Setup USART2: 8N1, DMA for TX and RX
    RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;
    USART2->CR1  = USART_CR1_RTOIE;
    USART2->CR2  = USART_CR2_RTOEN;
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
/// @brief  USART start frame transmit
/// @param  bytes_count: bytes count for transmit
/// @return none
//  ***************************************************************************
void usart2_start_tx(uint32_t bytes_count) {
    usart_reset(true, false);
    DMA1_Channel7->CMAR  = (uint32_t)tx_buffer;
    DMA1_Channel7->CNDTR = bytes_count;
    DMA1_Channel7->CCR  |= DMA_CCR_EN;
    USART2->CR1 |= USART_CR1_TE;
}

//  ***************************************************************************
/// @brief  USART start frame receive
/// @param  none
/// @return none
//  ***************************************************************************
void usart2_start_rx(void) {
    usart_reset(false, true);
    memset(rx_buffer, 0, sizeof(rx_buffer));
    DMA1_Channel6->CMAR  = (uint32_t)rx_buffer;
    DMA1_Channel6->CNDTR = sizeof(rx_buffer);
    DMA1_Channel6->CCR  |= DMA_CCR_EN;
    USART2->CR1 |= USART_CR1_RE;
}

//  ***************************************************************************
/// @brief  Get USART TX buffer address
/// @param  none
/// @return TX buffer address
//  ***************************************************************************
uint8_t* usart2_get_tx_buffer(void) {
    return tx_buffer;
}

//  ***************************************************************************
/// @brief  Get USART RX buffer address
/// @param  none
/// @return RX buffer address
//  ***************************************************************************
uint8_t* usart2_get_rx_buffer(void) {
    return rx_buffer;
}





//  ***************************************************************************
/// @brief  USART reset transmitted or\and receiver
/// @note   Clear interrupt flags according interrupt mapping diagram (figure 222 of Reference Manual)
/// @param  reset_tx: true - reset transmitter
/// @param  reset_rx: true - reset receiver
/// @return none
//  ***************************************************************************
static void usart_reset(bool reset_tx, bool reset_rx) {
    if (reset_tx) {
        USART2->CR1 &= ~USART_CR1_TE;
        USART2->ICR |= USART_ICR_FECF | USART_ICR_TCCF;
        DMA1_Channel7->CCR &= ~DMA_CCR_EN;
        DMA1->IFCR = DMA_IFCR_CGIF7;
    }
    if (reset_rx) {
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
        usart_callbacks.frame_error_callback();
    }
}

//  ***************************************************************************
/// @brief  DMA channel ISR for receiver
/// @param  none
/// @return none
//  ***************************************************************************
void DMA1_Channel6_IRQHandler(void) {
    uint32_t status = DMA1->ISR;
    if (status & DMA_ISR_TCIF6) { // DMA buffer is overflow
        usart_reset(false, true);
        usart_callbacks.frame_error_callback();
    }
    if (status & DMA_ISR_TEIF6) { // DMA memory access error
        usart_reset(false, true);
        usart_callbacks.frame_error_callback();
    }
}

//  ***************************************************************************
/// @brief  USART ISR
/// @param  none
/// @return none
//  ***************************************************************************
void USART2_IRQHandler(void) {
    uint32_t status = USART2->ISR;
    if (status & (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE | USART_ISR_PE)) {
        usart_reset(true, true);
        usart_callbacks.frame_error_callback();
    }
    if (status & USART_ISR_RTOF) {
        usart_reset(false, true);
        usart_callbacks.frame_received_callback(sizeof(rx_buffer) - DMA1_Channel6->CNDTR);
    }
}
