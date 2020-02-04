//  ***************************************************************************
/// @file    pwm.h
/// @author  NeoProg
/// @brief   Interface for 18-channel PWM driver
//  ***************************************************************************
#ifndef _PWM_H_
#define _PWM_H_

#include <stdint.h>
#include <stdbool.h>

#define SUPPORT_PWM_CHANNELS_COUNT                  (18)


// PWM period counter for synchronize
extern uint64_t synchro;


extern void pwm_init(void);
extern void pwm_enable(void);
extern void pwm_disable(void);
extern void pwm_set_shadow_buffer_state(bool is_locked);
extern void pwm_set_width(uint32_t channel, uint32_t width);


#endif // _PWM_H_


