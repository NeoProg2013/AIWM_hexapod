/// ***************************************************************************
/// @file    usart1.c
/// @author  NeoProg
/// ***************************************************************************
#include "usart1.h"
#include "project-base.h"
#define USART_TX_PIN                    GPIOA, 9
#define USART_RX_PIN                    GPIOA, 10

static uint8_t  tx_buffer[USART1_TX_BUFFER_SIZE] = {0};
static uint8_t  rx_buffer[512]  = {0};
static uint8_t* rx_buffer_cursor = NULL;
static uint32_t rx_bytes_count = 0;
static usart1_callbacks_t usart_callbacks;


static void usart_reset(bool reset_tx, bool reset_rx);


/// ***************************************************************************
/// @brief  USART initialization
/// @param  baud_rate: USART baud rate
/// @return none
/// ***************************************************************************
void usart1_init(uint32_t baud_rate, usart1_callbacks_t* callbacks) {
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
    
    // Setup USART: 8N1
    RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
    USART1->CR2  = USART_CR2_RTOEN;
    USART1->CR3  = USART_CR3_EIE;
    USART1->BRR  = SYSTEM_CLOCK_FREQUENCY / baud_rate;
    USART1->RTOR = 35; // 3.5 char timer
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, USART1_IRQ_PRIORITY);

    // Enable USART
    USART1->CR1 |= USART_CR1_UE;
    usart_reset(true, true);
}

/// ***************************************************************************
/// @brief  USART start frame transmit
/// @param  bytes_count: bytes count for transmit
/// @return none
/// ***************************************************************************
void usart1_start_sync_tx(uint32_t bytes_count) {
    if (bytes_count) {   
        usart_reset(true, false);
        USART1->CR1 |= USART_CR1_TE;
      
        uint8_t* buffer_cursor = tx_buffer;
        while (bytes_count > 0) {
            while ((USART1->ISR & USART_ISR_TXE) != USART_ISR_TXE);
            USART1->TDR = *buffer_cursor;
            --bytes_count;
            ++buffer_cursor;
        }
        while ((USART1->ISR & USART_ISR_TC) != USART_ISR_TC);
    }
    usart_reset(true, false);
}

/// ***************************************************************************
/// @brief  USART start frame receive
/// @param  none
/// @return none
/// ***************************************************************************
void usart1_start_rx(void) {
    usart_reset(false, true);
    memset(rx_buffer, 0, sizeof(rx_buffer));
    rx_bytes_count = 0;
    rx_buffer_cursor = rx_buffer;
    USART1->CR1 |= USART_CR1_RTOIE | USART_CR1_RXNEIE;
    USART1->CR1 |= USART_CR1_RE;
}

/// ***************************************************************************
/// @brief  Get USART TX buffer address
/// @param  none
/// @return TX buffer address
/// ***************************************************************************
uint8_t* usart1_get_tx_buffer(void) {
    return tx_buffer;
}

/// ***************************************************************************
/// @brief  Get USART RX buffer address
/// @param  none
/// @return RX buffer address
/// ***************************************************************************
uint8_t* usart1_get_rx_buffer(void) {
    return rx_buffer;
}





/// ***************************************************************************
/// @brief  USART reset transmitted or\and receiver
/// @note   Clear interrupt flags according interrupt mapping diagram (figure 222 of Reference Manual)
/// @param  reset_tx: true - reset TX
/// @param  reset_rx: true - reset RX
/// @return none
/// ***************************************************************************
static void usart_reset(bool reset_tx, bool reset_rx) {
    if (reset_tx) {
        USART1->CR1 &= ~USART_CR1_TE; // Disable TX
        USART1->ICR |= USART_ICR_FECF | USART_ICR_TCCF;
    }
    if (reset_rx) {
        USART1->CR1 &= ~USART_CR1_RE; // Disable RX
        USART1->CR1 &= ~(USART_CR1_RTOIE | USART_CR1_RXNEIE); // Disable interrupts
        USART1->ICR |= USART_ICR_RTOCF | USART_ICR_FECF | USART_ICR_NCF | USART_ICR_ORECF | USART_ICR_PECF;
    }
}





/// ***************************************************************************
/// @brief  USART ISR
/// @param  none
/// @return none
/// ***************************************************************************
void USART1_IRQHandler(void) {
    uint32_t status = USART1->ISR;
    if (status & (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE | USART_ISR_PE)) {
        usart_reset(false, true);
        usart_callbacks.frame_error_callback();
        return;
    }
    if ((status & USART_ISR_RTOF) && (USART1->CR1 & USART_CR1_RTOIE)) {
        usart_reset(false, true);
        usart_callbacks.frame_received_callback(rx_bytes_count);
    }
    if ((status & USART_ISR_RXNE) && (USART1->CR1 & USART_CR1_RXNEIE)) {
        if (rx_bytes_count < sizeof(rx_buffer)) {
            (*rx_buffer_cursor) = USART1->RDR;
            ++rx_bytes_count;
            ++rx_buffer_cursor;
        }
        else {
            USART1->RDR; // Dummy read
        }
    }
}
