//  ***************************************************************************
/// @file    usart1_queue.h
/// @author  NeoProg
/// @brief   Queue for USART1
//  ***************************************************************************
#ifndef _USART1_QUEUE_H_
#define _USART1_QUEUE_H_

#include <stdint.h>
#include <stdbool.h>


extern void usart1_queue_init(void);
extern bool usart1_queue_enqueue(uint8_t data);
extern bool usart1_queue_dequeue(uint8_t* data);
extern bool usart1_queue_is_empty(void);
extern void usart1_queue_clear(void);


#endif // _USART1_QUEUE_H_

