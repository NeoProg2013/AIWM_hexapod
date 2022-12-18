/// ***************************************************************************
/// @file    pwm.h
/// @author  NeoProg
/// @brief   Interface for 18-channel PWM driver
/// ***************************************************************************
#ifndef _PWM_H_
#define _PWM_H_


#define SUPPORT_PWM_CHANNELS_COUNT                  (18)

#define PWM_START_FREQUENCY_HZ                      (200)
#define PWM_MIN_FREQUENCY_HZ                        (60)
#define PWM_MAX_FREQUENCY_HZ                        (200)


extern void pwm_init(uint32_t frequency);
extern void pwm_set_state(bool is_enabled);
extern void pwm_set_frequency(uint32_t frequency);
extern void pwm_set_lock_state(bool is_locked);
extern bool pwm_is_ready(void);
extern void pwm_set_width(uint32_t channel, uint32_t width);


#endif // _PWM_H_
