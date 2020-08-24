//  ***************************************************************************
/// @file    usart2.h
/// @author  NeoProg
/// @brief   Interface for USART2 driver with using DMA
//  ***************************************************************************
#ifndef _USART2_H_
#define _USART2_H_

#include <stdint.h>
#include <stdbool.h>


typedef struct {
    void(*frame_received_callback)(uint32_t frame_size);
    void(*frame_transmitted_callback)(void);
    void(*error_callback)(void);
} usart2_callbacks_t;


extern void usart2_init(uint32_t baud_rate, usart2_callbacks_t* callbacks);
extern void usart2_start_tx(uint32_t bytes_count);
extern void usart2_start_rx(void);
extern uint8_t* usart2_get_tx_buffer(void);
extern uint8_t* usart2_get_rx_buffer(void);


#endif // _USART2_H_

