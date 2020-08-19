//  ***************************************************************************
/// @file    usart1.h
/// @author  NeoProg
/// @brief   Interface for USART1 driver
//  ***************************************************************************
#ifndef _USART1_H_
#define _USART1_H_

#include <stdint.h>
#include <stdbool.h>


extern void usart1_init(uint32_t baud_rate);
extern bool usart1_read(uint8_t* buffer, uint32_t count);
extern bool usart1_write(uint8_t* buffer, uint32_t count);


#endif // _USART2_H_

