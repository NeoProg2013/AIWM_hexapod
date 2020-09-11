//  ***************************************************************************
/// @file    usart1.h
/// @author  NeoProg
/// @brief   Interface for USART1 driver
//  ***************************************************************************
#ifndef _USART1_H_
#define _USART1_H_

#include <stdint.h>
#include <stdbool.h>


typedef struct {
    void(*frame_received_callback)(uint32_t frame_size);
    void(*frame_transmitted_callback)(void);
    void(*frame_error_callback)(void);
} usart1_callbacks_t;


extern void usart1_init(uint32_t baud_rate, usart1_callbacks_t* callbacks);
extern void usart1_start_tx(uint32_t bytes_count);
extern void usart1_start_rx(void);
extern uint8_t* usart1_get_tx_buffer(void);
extern uint8_t* usart1_get_rx_buffer(void);


#endif // _USART2_H_
