//  ***************************************************************************
/// @file    swlp.h
/// @author  NeoProg
/// @brief   Simple wireless protocol driver
//  ***************************************************************************
#ifndef _SWLP_H_
#define _SWLP_H_

#include <stdint.h>


extern void swlp_init(void);
extern uint32_t swlp_process_frame(const uint8_t* rx_buffer, uint32_t frame_size, uint8_t* tx_buffer);


#endif // _SWLP_H_
