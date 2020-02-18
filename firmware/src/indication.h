//  ***************************************************************************
/// @file    indication.h
/// @author  NeoProg
/// @brief   LEDs and Buzzer driver interface
//  ***************************************************************************
#ifndef _LEDS_H_
#define _LEDS_H_

#include <stdbool.h>


extern void indication_init(void);
extern void indication_set_light_state(bool is_enabled);
extern void indication_process(void);


#endif // _LEDS_H_
