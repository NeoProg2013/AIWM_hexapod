/// ***************************************************************************
/// @file    servo-driver.h
/// @author  NeoProg
/// @brief   Servo driver
/// ***************************************************************************
#ifndef _SERVO_DRIVER_H_
#define _SERVO_DRIVER_H_
#include "cli.h"

#define SUPPORT_SERVO_COUNT                         (18)


extern void servo_driver_init(void); 
extern void servo_driver_power_on(void);
extern void servo_driver_power_off(void);
extern void servo_driver_set_speed(uint32_t speed);
extern void servo_driver_move(uint32_t ch, float angle);
extern void servo_driver_process(void);

extern const cli_cmd_t* servo_get_cmd_list(uint32_t* count);


#endif // _SERVO_DRIVER_H_
