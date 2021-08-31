//  ***************************************************************************
/// @file    indication.h
/// @author  NeoProg
/// @brief   LEDs and Buzzer driver interface
//  ***************************************************************************
#ifndef _LEDS_H_
#define _LEDS_H_
#include <stdint.h>
#include "cli.h"


extern void indication_init(void);
extern void indication_process(void);

extern const cli_cmd_t* indication_get_cmd_list(uint32_t* count);


#endif // _LEDS_H_
