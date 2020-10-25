//  ***************************************************************************
/// @file    servo_driver.h
/// @author  NeoProg
/// @brief   Servo driver
//  ***************************************************************************
#ifndef _SERVO_DRIVER_H_
#define _SERVO_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "cli.h"


#define SUPPORT_SERVO_COUNT                        (18)


extern void servo_driver_init(void); 
extern void servo_driver_power_on(void);
extern void servo_driver_power_off(void);
extern void servo_driver_move(uint32_t ch, float angle);
extern void servo_driver_process(void);

extern bool servo_driver_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], 
                                             uint32_t argc, char* response);


#endif // _SERVO_DRIVER_H_
