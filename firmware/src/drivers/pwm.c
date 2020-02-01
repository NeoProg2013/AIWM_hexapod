//  ***************************************************************************
/// @file    pwm.c
/// @author  NeoProg
//  ***************************************************************************
#include "pwm.h"
#include "project_base.h"
#include "stm32f373xc.h"


#define SUPPORT_PWM_CHANNELS_COUNT      (18)
#define PWM_CHANNEL_DISABLE_VALUE       (0xFFFF)
#define PWM_CHANNEL_PULSE_TRIM          (3)

#define PWM_FREQUENCY_HZ                (160)
#define PWM_PERIOD_US                   (1000000 / PWM_FREQUENCY_HZ)

#if PWM_PERIOD_US > 65535
#error "PWM period should be less 65535, check PWM_FREQUENCY_HZ value"
#endif // PWM_PERIOD_US


typedef struct {
    uint32_t ticks;
    GPIO_TypeDef* gpio_port;
    uint32_t gpio_pin;
} pwm_channel_t;


static pwm_channel_t active_buffer[SUPPORT_PWM_CHANNELS_COUNT];     // Sorted buffer by ticks value. Using for load data to timer registers
static pwm_channel_t shadow_buffer[SUPPORT_PWM_CHANNELS_COUNT] = {  // Not sorted buffer. Using for write data from user.
	{ .gpio_port = GPIOD, .gpio_pin =  8, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOB, .gpio_pin = 15, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOB, .gpio_pin = 14, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOE, .gpio_pin =  9, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOE, .gpio_pin =  8, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOB, .gpio_pin =  2, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOB, .gpio_pin =  1, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOB, .gpio_pin =  0, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOC, .gpio_pin =  5, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOC, .gpio_pin =  4, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  7, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  6, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  5, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  4, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  3, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  2, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  1, .ticks = PWM_CHANNEL_DISABLE_VALUE },
	{ .gpio_port = GPIOA, .gpio_pin =  0, .ticks = PWM_CHANNEL_DISABLE_VALUE }
};
static bool shadow_buffer_is_lock = false;
static bool pwm_disable_is_requested = false;

uint64_t synchro = 0;


//  ***************************************************************************
/// @brief  PWM driver initialization
/// @param  none
/// @return none
//  ***************************************************************************
void pwm_init(void) {

    // Initialization active buffer and configure GPIO
    for (uint32_t i = 0; i < SUPPORT_PWM_CHANNELS_COUNT; ++i) {

		active_buffer[i] = shadow_buffer[i];

		active_buffer[i].gpio_port->BRR      =  (0x01 << (active_buffer[i].gpio_pin * 1));   // Reset output
		active_buffer[i].gpio_port->MODER   |=  (0x01 << (active_buffer[i].gpio_pin * 2));   // Output mode
		active_buffer[i].gpio_port->OSPEEDR |=  (0x03 << (active_buffer[i].gpio_pin * 2));   // High speed
		active_buffer[i].gpio_port->PUPDR   &= ~(0x03 << (active_buffer[i].gpio_pin * 2));   // No pull-up, no pull-down
	}

    // Configure PWM timer clocks
    RCC->APB2ENR  |= RCC_APB2ENR_TIM17EN;
    RCC->APB2RSTR |= RCC_APB2RSTR_TIM17RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM17RST;

    // Configure PWM timer: one pulse mode
    TIM17->CR1   = TIM_CR1_OPM | TIM_CR1_URS; 
    TIM17->DIER  = TIM_DIER_CC1IE | TIM_DIER_UIE;
    TIM17->PSC   = APB2_CLOCK_FREQUENCY / 1000000 - 1; // 1 tick = 1 us
    TIM17->ARR   = PWM_PERIOD_US;
    NVIC_EnableIRQ(TIM17_IRQn);
    NVIC_SetPriority(TIM17_IRQn, TIM17_IRQ_PRIORITY);
}

//  ***************************************************************************
/// @brief  PWM enable
/// @param  none
/// @return none
//  ***************************************************************************
void pwm_enable(void) {

    pwm_disable_is_requested = false;

    // Reset and enable timer if it is not started
    if ((TIM17->CR1 & TIM_CR1_CEN) == 0) {
        TIM17->CCR1  = PWM_CHANNEL_DISABLE_VALUE;
        TIM17->CR1  |= TIM_CR1_CEN;
    }
}

//  ***************************************************************************
/// @brief  PWM disable
/// @note   PWM will be disable on next PWM period
/// @param  none
/// @return none
//  ***************************************************************************
void pwm_disable(void) {

    pwm_disable_is_requested = true;
}

//  ***************************************************************************
/// @brief  Lock shadow buffer
/// @note   If buffer is lock, then data from shadow buffer not load to active
/// @param  is_locked: true - buffer is lock, false - buffer is unlock
/// @return none
//  ***************************************************************************
void pwm_set_shadow_buffer_state(bool is_locked) {

    shadow_buffer_is_lock = is_locked;
}

//  ***************************************************************************
/// @brief  Set PWM channel pulse width
/// @param  channel: PWM channel index
/// @param  width: pulse width
/// @return none
//  ***************************************************************************
void pwm_set_width(uint32_t channel, uint32_t width) {
    
    int32_t ticks = width - PWM_CHANNEL_PULSE_TRIM;
    if (ticks < 0) {
        ticks = 0;
    }
    
    shadow_buffer[channel].ticks = ticks;
}





//  ***************************************************************************
/// @brief  PWM timer ISR
/// @param  none
/// @return none
//  ***************************************************************************
void TIM17_IRQHandler(void) {
    
    DEBUG_TP1_PIN_SET;

    static uint32_t ch_cursor = 0;

    // Read and clear status register
    uint32_t status = TIM17->SR;
    TIM17->SR = 0;

    //
    // Process events
    //
    if (status & TIM_SR_CC1IF) {

        while (ch_cursor < SUPPORT_PWM_CHANNELS_COUNT) {

            // Find equal time for PWM outputs. First iteration not should be pass this check
            if (TIM17->CCR1 != active_buffer[ch_cursor].ticks) {
                TIM17->CCR1 = active_buffer[ch_cursor].ticks; // Load time for next PWM output
                break;
            }

            // Set LOW level for PWM output
            active_buffer[ch_cursor].gpio_port->BRR = (0x01 << active_buffer[ch_cursor].gpio_pin);

            // Go to next PWM channel
            ++ch_cursor;
        }
    }
    if (status & TIM_SR_UIF) {  // We are reached end of PWM period

        // Check disable PWM request
        if (pwm_disable_is_requested == true) {
            return;
        }

        // Sync shadow buffer with active buffer if it unlock
        if (shadow_buffer_is_lock == false) {
            for (uint32_t i = 0; i < SUPPORT_PWM_CHANNELS_COUNT; ++i) {
                active_buffer[i] = shadow_buffer[i];
            }
        }

        // Sorting PWM channels: bubble sorting method
        for (uint32_t i = 0; i < SUPPORT_PWM_CHANNELS_COUNT - 1; ++i) {

            for (uint32_t j = 0; j < SUPPORT_PWM_CHANNELS_COUNT - i - 1; ++j) {

                if (active_buffer[j].ticks > active_buffer[j + 1].ticks) {
                    pwm_channel_t temp = active_buffer[j];
                    active_buffer[j] = active_buffer[j + 1];
                    active_buffer[j + 1] = temp;
                }
            }
        }

        // Set HIGH level for PWM outputs
        for (uint32_t i = 0; i < SUPPORT_PWM_CHANNELS_COUNT; ++i) {
            active_buffer[i].gpio_port->BSRR = (0x01 << active_buffer[i].gpio_pin);
        }
        
        // Reset and enable PWM timer
        ch_cursor = 0;
        TIM17->CCR1 = active_buffer[ch_cursor].ticks;
        TIM17->CR1 |= TIM_CR1_CEN;

        ++synchro;
    }
    
    DEBUG_TP1_PIN_CLR;
}
