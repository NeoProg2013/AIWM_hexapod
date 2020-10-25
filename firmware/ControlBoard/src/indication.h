//  ***************************************************************************
/// @file    indication.h
/// @author  NeoProg
/// @brief   LEDs and Buzzer driver interface
//  ***************************************************************************
#ifndef _LEDS_H_
#define _LEDS_H_

#include <stdint.h>
#include <stdbool.h>
#include "cli.h"


extern void indication_init(void);
extern void indication_process(void);

extern bool indication_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response);


#endif // _LEDS_H_
