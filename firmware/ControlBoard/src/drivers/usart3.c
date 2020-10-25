//  ***************************************************************************
/// @file    usart3.c
/// @author  NeoProg
//  ***************************************************************************
#include "usart3.h"
#include "project_base.h"

#define USART_RX_PIN                    (11) // PC11


static uint8_t  rx_buffer[32]  = {0};
static uint8_t* rx_buffer_cursor = NULL;
static uint32_t rx_bytes_count = 0;
static usart3_callbacks_t usart_callbacks;


static void usart_reset(void);


//  ***************************************************************************
/// @brief  USART initialization
/// @param  baud_rate: USART baud rate
/// @return none
//  ***************************************************************************
void usart3_init(uint32_t baud_rate, usart3_callbacks_t* callbacks) {
    
    usart_callbacks = *callbacks;
    
    //
    // Setup GPIO
    //
    // Setup RX pin
    GPIOC->MODER   |=  (0x02u << (USART_RX_PIN * 2u)); // Alternate function mode
    GPIOC->OSPEEDR |=  (0x03u << (USART_RX_PIN * 2u)); // High speed
    GPIOC->PUPDR   &= ~(0x03u << (USART_RX_PIN * 2u)); // Disable pull
    GPIOC->PUPDR   |=  (0x01u << (USART_RX_PIN * 2u)); // Enable pull up
    GPIOC->AFR[1]  |=  (0x07u << (USART_RX_PIN * 4u - 32)); // AF7
    
    
    //
    // Setup USART
    //
    RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USART3RST;

    // Setup USART: 8N1
    USART3->CR2  = USART_CR2_RTOEN;
    USART3->CR3  = USART_CR3_EIE;
    USART3->BRR  = SYSTEM_CLOCK_FREQUENCY / baud_rate;
    USART3->RTOR = 35; // 3.5 char timer
    NVIC_EnableIRQ(USART3_IRQn);
    NVIC_SetPriority(USART3_IRQn, USART3_IRQ_PRIORITY);

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
    rx_bytes_count = 0;
    rx_buffer_cursor = rx_buffer;
    USART3->CR1 |= USART_CR1_RTOIE | USART_CR1_RXNEIE;
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
    USART3->CR1 &= ~USART_CR1_RE; // Disable RX
    USART3->CR1 &= ~(USART_CR1_RTOIE | USART_CR1_RXNEIE); // Disable interrupts
    USART3->ICR |= USART_ICR_RTOCF | USART_ICR_FECF | USART_ICR_NCF | USART_ICR_ORECF | USART_ICR_PECF;
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
    if ((status & USART_ISR_RTOF) && (USART3->CR1 & USART_CR1_RTOIE)) {
        usart_reset();
        usart_callbacks.frame_received_callback(rx_bytes_count);
    }
    if ((status & USART_ISR_RXNE) && (USART3->CR1 & USART_CR1_RXNEIE)) {
        if (rx_bytes_count < sizeof(rx_buffer)) {
            (*rx_buffer_cursor) = USART3->RDR;
            ++rx_bytes_count;
            ++rx_buffer_cursor;
        }
        else {
            USART3->RDR; // Dummy read
        } 
    }
}
