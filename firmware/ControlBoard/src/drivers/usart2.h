//  ***************************************************************************
/// @file    usart2.h
/// @author  NeoProg
/// @brief   Interface for USART2 driver with using DMA
//  ***************************************************************************
#ifndef _USART2_H_
#define _USART2_H_

#include <stdint.h>


typedef struct {
    void(*frame_received_callback)(uint32_t frame_size);
    void(*frame_transmitted_callback)(void);
    void(*error_callback)(void);
} usart2_callbacks_t;


extern void usart2_init(uint32_t baud_rate, usart2_callbacks_t* callbacks);
extern void usart2_start_tx(uint8_t* tx_buffer, uint32_t bytes_count);
extern void usart2_start_rx(uint8_t* rx_buffer, uint32_t buffer_size);


#endif // _USART2_H_

