/// ***************************************************************************
/// @file    usart3.h
/// @author  NeoProg
/// @brief   Interface for USART3 (RX only)
/// ***************************************************************************
#ifndef _USART3_H_
#define _USART3_H_

#include <stdint.h>
#include <stdbool.h>


typedef struct {
    void(*frame_received_callback)(uint32_t frame_size);
    void(*frame_error_callback)(void);
} usart3_callbacks_t;


extern void usart3_init(uint32_t baud_rate, usart3_callbacks_t* callbacks);
extern void usart3_start_rx(void);
extern uint8_t* usart3_get_rx_buffer(void);


#endif // _USART3_H_
