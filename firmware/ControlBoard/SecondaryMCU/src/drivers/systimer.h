//  ***************************************************************************
/// @file    systimer.h
/// @author  NeoProg
/// @brief   System timer (1 kHz)
//  ***************************************************************************
#ifndef _SYSTIMER_H_
#define _SYSTIMER_H_

#include <stdint.h>


extern void systimer_init(void);
extern void systimer_change_step(uint32_t step_ms);
extern uint64_t get_time_ms(void);
extern void delay_ms(uint32_t ms);


#endif // _SYSTIMER_H_
