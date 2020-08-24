//  ***************************************************************************
/// @file    usart1.c
/// @author  NeoProg
//  ***************************************************************************
#include "usart1.h"
#include "project_base.h"

#define USART_TX_PIN                    (9)  // PA9
#define USART_RX_PIN                    (10) // PA10


static uint8_t  tx_buffer[3072] = {0};
static uint8_t  rx_buffer[512]  = {0};
static uint8_t* tx_buffer_cursor = NULL;
static uint32_t tx_bytes_count = 0;
static uint8_t* rx_buffer_cursor = NULL;
static uint32_t rx_bytes_count = 0;
static usart1_callbacks_t usart_callbacks;


static void usart_reset(bool reset_tx, bool reset_rx);


//  ***************************************************************************
/// @brief  USART initialization
/// @param  baud_rate: USART baud rate
/// @return none
//  ***************************************************************************
void usart1_init(uint32_t baud_rate, usart1_callbacks_t* callbacks) {
    
    usart_callbacks = *callbacks;
    
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

    // Setup USART: 8N2
    USART1->CR2  = USART_CR2_RTOEN | USART_CR2_STOP_1;
    USART1->CR3  = USART_CR3_EIE;
    USART1->BRR  = SYSTEM_CLOCK_FREQUENCY / baud_rate;
    USART1->RTOR = 35; // 3.5 char timer
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, USART1_IRQ_PRIORITY);

    // Enable USART
    USART1->CR1 |= USART_CR1_UE;
    usart_reset(true, true);
}

//  ***************************************************************************
/// @brief  USART start frame transmit
/// @param  bytes_count: bytes count for transmit
/// @return none
//  ***************************************************************************
void usart1_start_tx(uint32_t bytes_count) {
    if (bytes_count) {  
        usart_reset(true, false);
        tx_bytes_count = bytes_count;
        tx_buffer_cursor = tx_buffer;
        USART1->CR1 |= USART_CR1_TXEIE;
        USART1->CR1 |= USART_CR1_TE;
    }
}

//  ***************************************************************************
/// @brief  USART start frame receive
/// @param  none
/// @return none
//  ***************************************************************************
void usart1_start_rx(void) {
    usart_reset(false, true);
    memset(rx_buffer, 0, sizeof(rx_buffer));
    rx_bytes_count = 0;
    rx_buffer_cursor = rx_buffer;
    USART1->CR1 |= USART_CR1_RTOIE | USART_CR1_RXNEIE;
    USART1->CR1 |= USART_CR1_RE;
}

//  ***************************************************************************
/// @brief  Get USART TX buffer address
/// @param  none
/// @return TX buffer address
//  ***************************************************************************
uint8_t* usart1_get_tx_buffer(void) {
    return tx_buffer;
}

//  ***************************************************************************
/// @brief  Get USART RX buffer address
/// @param  none
/// @return RX buffer address
//  ***************************************************************************
uint8_t* usart1_get_rx_buffer(void) {
    return rx_buffer;
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
        USART1->CR1 &= ~USART_CR1_TE; // Disable TX
        USART1->CR1 &= ~(USART_CR1_TCIE | USART_CR1_TXEIE); // Disable interrupts
        USART1->ICR |= USART_ICR_FECF | USART_ICR_TCCF;
    }
        
    // Reset RX
    if (reset_rx) {
        USART1->CR1 &= ~USART_CR1_RE; // Disable RX
        USART1->CR1 &= ~(USART_CR1_RTOIE | USART_CR1_RXNEIE); // Disable interrupts
        USART1->ICR |= USART_ICR_RTOCF | USART_ICR_FECF | USART_ICR_NCF | USART_ICR_ORECF | USART_ICR_PECF;
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
        if (usart_callbacks.error_callback) {
            usart_callbacks.error_callback();
        }
    }
    if ((status & USART_ISR_RTOF) && (USART1->CR1 & USART_CR1_RTOIE)) {
        usart_reset(false, true);
        if (usart_callbacks.frame_received_callback) {
            usart_callbacks.frame_received_callback(rx_bytes_count);
        }
    }
    if ((status & USART_ISR_RXNE) && (USART1->CR1 & USART_CR1_RXNEIE)) {
        (*rx_buffer_cursor) = USART1->RDR;
        ++rx_bytes_count;
        ++rx_buffer_cursor;
        if (rx_bytes_count > sizeof(rx_buffer)) {
            usart_reset(false, true);
        }
    }
    if ((status & USART_ISR_TXE) && (USART1->CR1 & USART_CR1_TXEIE)) {
        USART1->TDR = (*tx_buffer_cursor);
        --tx_bytes_count;
        ++tx_buffer_cursor;
        if (tx_bytes_count == 0) {
            USART1->CR1 &= ~USART_CR1_TXEIE;
            USART1->CR1 |= USART_CR1_TCIE;
        }
    }
    if ((status & USART_ISR_TC) && (USART1->CR1 & USART_CR1_TCIE)) {
        usart_reset(true, false);
        if (usart_callbacks.frame_transmitted_callback) {
            usart_callbacks.frame_transmitted_callback();
        }
    }
}
