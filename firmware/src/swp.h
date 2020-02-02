//  ***************************************************************************
/// @file    swp.h
/// @author  NeoProg
/// @brief   Simple wireless protocol driver
//  ***************************************************************************
#ifndef _SWP_H_
#define _SWP_H_

#include <stdint.h>


extern void swp_init(void);
extern uint32_t swp_process_frame(const uint8_t* rx_buffer, uint32_t frame_size, uint8_t* tx_buffer);


#endif // _SWP_H_
