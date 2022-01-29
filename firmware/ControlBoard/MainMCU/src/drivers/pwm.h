/// ***************************************************************************
/// @file    pwm.h
/// @author  NeoProg
/// @brief   Interface for 18-channel PWM driver
/// ***************************************************************************
#ifndef _PWM_H_
#define _PWM_H_

#include <stdint.h>
#include <stdbool.h>

#define SUPPORT_PWM_CHANNELS_COUNT                  (18)

#define PWM_START_FREQUENCY_HZ                      (200)
#define PWM_MIN_FREQUENCY_HZ                        (60)
#define PWM_MAX_FREQUENCY_HZ                        (200)


// PWM period counter for synchronize
extern uint64_t synchro;


extern void pwm_init(uint32_t frequency);
extern void pwm_enable(void);
extern void pwm_disable(void);
extern void pwm_set_frequency(uint32_t frequency);
extern void pwm_set_shadow_buffer_lock_state(bool is_locked);
extern void pwm_set_width(uint32_t channel, uint32_t width);


#endif // _PWM_H_
