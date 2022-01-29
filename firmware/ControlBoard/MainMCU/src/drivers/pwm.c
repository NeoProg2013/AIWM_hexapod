    /// ***************************************************************************
/// @file    pwm.c
/// @author  NeoProg
/// ***************************************************************************
#include "pwm.h"
#include "stm32f373xc.h"
#include "project-base.h"
#include <string.h>
#define PWM_CHANNEL_DISABLE_VALUE       (0xFFFF)
#define PWM_CHANNEL_PULSE_TRIM          (3)

#if 1000000 / PWM_MIN_FREQUENCY_HZ > 65535
#error "PWM period should be less 65535 ticks (1 tick = 1us), check PWM_START_FREQUENCY_HZ value"
#endif
#if 1000000 / PWM_MAX_FREQUENCY_HZ < 3000
#error "PWM period should be more 3000 ticks (1 tick = 1us), check PWM_START_FREQUENCY_HZ value"
#endif


typedef struct {
    uint32_t ticks;
    GPIO_TypeDef* gpio_port;
    uint32_t gpio_pin;
} pwm_channel_t;

// Active array of pointers to active buffer (sorted)
// To this buffer can access from IRQ handler
static pwm_channel_t* active_buffer_ptr[SUPPORT_PWM_CHANNELS_COUNT]; 

static pwm_channel_t active_buffer[SUPPORT_PWM_CHANNELS_COUNT];      // PWM active buffer. It is copy of shadow buffer, but it has other RAM address
static pwm_channel_t shadow_buffer[SUPPORT_PWM_CHANNELS_COUNT] = {   // Not sorted buffer. Using for write data from user.
    { .gpio_port = GPIOD, .gpio_pin =  8, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOB, .gpio_pin = 15, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOB, .gpio_pin = 14, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOE, .gpio_pin =  9, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOE, .gpio_pin =  8, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOB, .gpio_pin =  2, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOB, .gpio_pin =  1, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOB, .gpio_pin =  0, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOC, .gpio_pin =  5, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    
    { .gpio_port = GPIOA, .gpio_pin =  0, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  1, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  2, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  3, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  4, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  5, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  6, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOA, .gpio_pin =  7, .ticks = PWM_CHANNEL_DISABLE_VALUE },
    { .gpio_port = GPIOC, .gpio_pin =  4, .ticks = PWM_CHANNEL_DISABLE_VALUE },
};
static bool shadow_buffer_is_locked = false;
static bool pwm_disable_is_requested = false;

// Next value load to current value each PWM period
static uint32_t pwm_current_frequency = PWM_START_FREQUENCY_HZ;
static uint32_t pwm_next_frequency = PWM_START_FREQUENCY_HZ;

// Pre-calculated GPIOx BSR register values
static uint16_t pwm_gpio_a_bsr = 0;
static uint16_t pwm_gpio_b_bsr = 0;
static uint16_t pwm_gpio_c_bsr = 0;
static uint16_t pwm_gpio_d_bsr = 0;
static uint16_t pwm_gpio_e_bsr = 0;

uint64_t synchro = 0;


/// ***************************************************************************
/// @brief  PWM driver initialization
/// @param  frequency: frequency [Hz]
/// @return none
/// ***************************************************************************
void pwm_init(uint32_t frequency) {
    pwm_current_frequency = frequency;
    pwm_next_frequency = frequency;
    
    // Initialization active buffer
    for (uint32_t i = 0; i < SUPPORT_PWM_CHANNELS_COUNT; ++i) {
        active_buffer[i] = shadow_buffer[i];
        active_buffer_ptr[i] = &active_buffer[i];
    }

    // Setup GPIO
    for (uint32_t i = 0; i < SUPPORT_PWM_CHANNELS_COUNT; ++i) {
        gpio_reset           (active_buffer[i].gpio_port, active_buffer[i].gpio_pin);
        gpio_set_mode        (active_buffer[i].gpio_port, active_buffer[i].gpio_pin, GPIO_MODE_OUTPUT);
        gpio_set_output_type (active_buffer[i].gpio_port, active_buffer[i].gpio_pin, GPIO_TYPE_PUSH_PULL);
        gpio_set_output_speed(active_buffer[i].gpio_port, active_buffer[i].gpio_pin, GPIO_SPEED_HIGH);
        gpio_set_pull        (active_buffer[i].gpio_port, active_buffer[i].gpio_pin, GPIO_PULL_NO);
        
        if (active_buffer[i].gpio_port == GPIOA) pwm_gpio_a_bsr |= (0x01 << active_buffer[i].gpio_pin);
        if (active_buffer[i].gpio_port == GPIOB) pwm_gpio_b_bsr |= (0x01 << active_buffer[i].gpio_pin);
        if (active_buffer[i].gpio_port == GPIOC) pwm_gpio_c_bsr |= (0x01 << active_buffer[i].gpio_pin);
        if (active_buffer[i].gpio_port == GPIOD) pwm_gpio_d_bsr |= (0x01 << active_buffer[i].gpio_pin);
        if (active_buffer[i].gpio_port == GPIOE) pwm_gpio_e_bsr |= (0x01 << active_buffer[i].gpio_pin);
    }

    // Setup PWM timer: one pulse mode
    RCC->APB2RSTR |= RCC_APB2RSTR_TIM17RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM17RST;
    TIM17->CR1  = TIM_CR1_OPM | TIM_CR1_URS; 
    TIM17->DIER = TIM_DIER_CC1IE | TIM_DIER_UIE;
    TIM17->PSC  = APB2_CLOCK_FREQUENCY / 1000000 - 1; // 1 tick = 1 us
    TIM17->ARR  = 1000000 / pwm_current_frequency; // Period
    NVIC_EnableIRQ(TIM17_IRQn);
    NVIC_SetPriority(TIM17_IRQn, TIM17_IRQ_PRIORITY);
}

/// ***************************************************************************
/// @brief  PWM enable
/// @param  none
/// @return none
/// ***************************************************************************
void pwm_enable(void) {
    pwm_disable_is_requested = false;

    // Reset and enable timer if it is not started
    if ((TIM17->CR1 & TIM_CR1_CEN) == 0) {
        TIM17->CCR1 = PWM_CHANNEL_DISABLE_VALUE;
        TIM17->CR1 |= TIM_CR1_CEN;
    }
}

/// ***************************************************************************
/// @brief  PWM disable
/// @note   PWM will be disable on next PWM period
/// @param  none
/// @return none
/// ***************************************************************************
void pwm_disable(void) {
    pwm_disable_is_requested = true;
}

/// ***************************************************************************
/// @brief  Set PWM frequency
/// @param  frequency: frequency [Hz]
/// @return none
/// ***************************************************************************
void pwm_set_frequency(uint32_t frequency) {
    if (frequency < PWM_MIN_FREQUENCY_HZ) {
        frequency = PWM_MIN_FREQUENCY_HZ;
    }
    if (frequency > PWM_MAX_FREQUENCY_HZ) {
        frequency = PWM_MAX_FREQUENCY_HZ;
    }
    pwm_next_frequency = frequency;
}

/// ***************************************************************************
/// @brief  Lock shadow buffer
/// @note   If buffer is lock, then data from shadow buffer not load to active
/// @param  is_locked: true - buffer is lock, false - buffer is unlock
/// @return none
/// ***************************************************************************
void pwm_set_shadow_buffer_lock_state(bool is_locked) {
    shadow_buffer_is_locked = is_locked;
}

/// ***************************************************************************
/// @brief  Set PWM channel pulse width
/// @param  channel: PWM channel index
/// @param  width: pulse width
/// @return none
/// ***************************************************************************
void pwm_set_width(uint32_t channel, uint32_t width) {
    int32_t ticks = (int32_t)width - PWM_CHANNEL_PULSE_TRIM;
    if (ticks < 0) {
        ticks = 0;
    }
    shadow_buffer[channel].ticks = ticks;
}





/// ***************************************************************************
/// @brief  PWM timer ISR
/// @param  none
/// @return none
/// ***************************************************************************
int compare_channels(const void* ch1, const void* ch2) {
    uint16_t ch1_ticks = ((pwm_channel_t*)ch1)->ticks;
    uint16_t ch2_ticks = ((pwm_channel_t*)ch2)->ticks;
    if (ch1_ticks > ch2_ticks) return  1;
    if (ch1_ticks < ch2_ticks) return -1;
    return 0;
}
void TIM17_IRQHandler(void) {
    static uint32_t ch_cursor = 0;

    // Read and clear status register
    uint32_t status = TIM17->SR;
    TIM17->SR = 0;
    
    if (status & TIM_SR_CC1IF) {
        TIM17->CR1 &= ~TIM_CR1_CEN;
        while (ch_cursor < SUPPORT_PWM_CHANNELS_COUNT) {
            if (active_buffer_ptr[ch_cursor]->ticks > TIM17->CCR1) {
                TIM17->CCR1 = active_buffer_ptr[ch_cursor]->ticks; // Load time for next PWM output
                break;
            }
            
            // Set LOW level for PWM output
            gpio_reset(active_buffer_ptr[ch_cursor]->gpio_port, active_buffer_ptr[ch_cursor]->gpio_pin);
            
            // Go to next PWM channel
            ++ch_cursor;
        }
        TIM17->CR1 |= TIM_CR1_CEN;
    }
    if (status & TIM_SR_UIF) {  // We are reached end of PWM period
        // Check disable PWM request
        if (pwm_disable_is_requested) {
            return;
        }

        // Sync shadow buffer with active buffer if it unlocked
        if (shadow_buffer_is_locked == false) {
            memcpy(active_buffer, shadow_buffer, sizeof(active_buffer));
        }

        // Sorting PWM channels
        qsort(active_buffer_ptr, SUPPORT_PWM_CHANNELS_COUNT, sizeof(active_buffer_ptr[0]), compare_channels);

        // Set HIGH level for PWM outputs
        GPIOA->BSRR = pwm_gpio_a_bsr;
        GPIOB->BSRR = pwm_gpio_b_bsr;
        GPIOC->BSRR = pwm_gpio_c_bsr;
        GPIOD->BSRR = pwm_gpio_d_bsr;
        GPIOE->BSRR = pwm_gpio_e_bsr;
        
        // Reset and enable PWM timer
        ch_cursor = 0;
        pwm_current_frequency = pwm_next_frequency;
        TIM17->ARR  = 1000000 / pwm_current_frequency;
        TIM17->CCR1 = active_buffer_ptr[ch_cursor]->ticks;
        TIM17->CR1 |= TIM_CR1_CEN;

        ++synchro;
    }
}
