//  ***************************************************************************
/// @file    usart3.c
/// @author  NeoProg
//  ***************************************************************************
#include "usart3.h"
#include "project-base.h"
#define USART_RX_PIN                    GPIOC, 11

static uint8_t rx_buffer[32]  = {0};
static usart3_callbacks_t usart_callbacks;


static void usart_reset(void);


//  ***************************************************************************
/// @brief  USART initialization
/// @param  baud_rate: USART baud rate
/// @return none
//  ***************************************************************************
void usart3_init(uint32_t baud_rate, usart3_callbacks_t* callbacks) {
    usart_callbacks = *callbacks;
    
    // Setup RX pin
    gpio_set_mode        (USART_RX_PIN, GPIO_MODE_AF);
    gpio_set_output_speed(USART_RX_PIN, GPIO_SPEED_HIGH);
    gpio_set_pull        (USART_RX_PIN, GPIO_PULL_UP);
    gpio_set_af          (USART_RX_PIN, 7);
    
    // Setup USART: 8N1, DMA for RX
    RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USART3RST;
    USART3->CR1  = USART_CR1_RTOIE;
    USART3->CR2  = USART_CR2_RTOEN;
    USART3->CR3  = USART_CR3_DMAR | USART_CR3_EIE;
    USART3->BRR  = SYSTEM_CLOCK_FREQUENCY / baud_rate;
    USART3->RTOR = 35; // 3.5 char timer
    NVIC_EnableIRQ(USART3_IRQn);
    NVIC_SetPriority(USART3_IRQn, USART3_IRQ_PRIORITY);
    
    // Setup DMA channel for RX
    DMA1_Channel3->CCR  &= ~DMA_CCR_EN;
    DMA1_Channel3->CCR   = DMA_CCR_MINC | DMA_CCR_TEIE | DMA_CCR_TCIE;
    DMA1_Channel3->CPAR  = (uint32_t)(&USART3->RDR);
    DMA1_Channel3->CMAR  = 0;
    DMA1_Channel3->CNDTR = 0;
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);
    NVIC_SetPriority(DMA1_Channel3_IRQn, USART3_IRQ_PRIORITY);

    // Enable USART
    USART3->CR1 |= USART_CR1_UE;
    usart_reset();
}

//  ***************************************************************************
/// @brief  USART start frame receive
/// @param  none
/// @return none
//  ***************************************************************************
void usart3_start_rx(void) {
    usart_reset();
    memset(rx_buffer, 0, sizeof(rx_buffer));
    DMA1_Channel3->CMAR  = (uint32_t)rx_buffer;
    DMA1_Channel3->CNDTR = sizeof(rx_buffer);
    DMA1_Channel3->CCR  |= DMA_CCR_EN;
    USART3->CR1 |= USART_CR1_RE;
}

//  ***************************************************************************
/// @brief  Get USART RX buffer address
/// @param  none
/// @return RX buffer address
//  ***************************************************************************
uint8_t* usart3_get_rx_buffer(void) {
    return rx_buffer;
}





//  ***************************************************************************
/// @brief  USART reset receiver
/// @note   Clear interrupt flags according interrupt mapping diagram (figure 222 of Reference Manual)
/// @return none
//  ***************************************************************************
static void usart_reset(void) {
    USART3->CR1 &= ~USART_CR1_RE;
    USART3->ICR |= USART_ICR_RTOCF | USART_ICR_FECF | USART_ICR_NCF | USART_ICR_ORECF | USART_ICR_PECF;
    DMA1_Channel3->CCR &= ~DMA_CCR_EN;
    DMA1->IFCR = DMA_IFCR_CGIF3;
}





//  ***************************************************************************
/// @brief  DMA channel ISR for receiver
/// @param  none
/// @return none
//  ***************************************************************************
void DMA1_Channel3_IRQHandler(void) {
    uint32_t status = DMA1->ISR;
    if (status & DMA_ISR_TCIF3) { // DMA buffer is overflow
        usart_reset();
        usart_callbacks.frame_error_callback();
    }
    if (status & DMA_ISR_TEIF3) { // DMA memory access error
        usart_reset();
        usart_callbacks.frame_error_callback();
    }
}

//  ***************************************************************************
/// @brief  USART ISR
/// @param  none
/// @return none
//  ***************************************************************************
void USART3_IRQHandler(void) {
    uint32_t status = USART3->ISR;
    if (status & (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE | USART_ISR_PE)) {
        usart_reset();
        usart_callbacks.frame_error_callback();
        return;
    }
    if (status & USART_ISR_RTOF) {
        usart_reset();
        usart_callbacks.frame_received_callback(sizeof(rx_buffer) - DMA1_Channel3->CNDTR);
    }
}
