/// ***************************************************************************
/// @file    pwm.c
/// @author  NeoProg
/// ***************************************************************************
#include "project-base.h"
#include "pwm.h"
#include "system-monitor.h"

static_assert(1000000 / PWM_MIN_FREQUENCY_HZ <= 65535, "PWM period should be less 65535 ticks (1 tick = 1us), check PWM_MIN_FREQUENCY_HZ value");
static_assert(1000000 / PWM_MAX_FREQUENCY_HZ >= 3000, "PWM period should be more 3000 ticks (1 tick = 1us), check PWM_MAX_FREQUENCY_HZ value");

#define PWM_CHANNEL_DISABLE_VALUE           (0xFFFF)


typedef struct {
    uint32_t ticks;
    GPIO_TypeDef* gpio_port;
    uint32_t gpio_pin;
} pwm_channel_t;

static int compare_channels(const void* a, const void* b);

// Active array of pointers to channels (sorted)
// To this buffer can access from IRQ handler
static pwm_channel_t* pwm_channels_ptr[SUPPORT_PWM_CHANNELS_COUNT]; 
static pwm_channel_t pwm_channels[SUPPORT_PWM_CHANNELS_COUNT] = {
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
static uint32_t pwm_frequency = PWM_START_FREQUENCY_HZ;
static bool pwm_locked = false;
static bool pwm_ready = false;
static bool pwm_sync_enabled = false;

// Pre-calculated GPIOx BSR register values
static uint16_t pwm_gpio_a_bsr = 0;
static uint16_t pwm_gpio_b_bsr = 0;
static uint16_t pwm_gpio_c_bsr = 0;
static uint16_t pwm_gpio_d_bsr = 0;
static uint16_t pwm_gpio_e_bsr = 0;


/// ***************************************************************************
/// @brief  PWM driver initialization
/// @param  frequency: frequency [Hz]
/// ***************************************************************************
void pwm_init(uint32_t frequency) {
    pwm_frequency = frequency;
    
    // Initialization active buffer
    for (uint32_t i = 0; i < SUPPORT_PWM_CHANNELS_COUNT; ++i) {
        pwm_channels_ptr[i] = &pwm_channels[i];
    }

    // Setup GPIO
    for (uint32_t i = 0; i < SUPPORT_PWM_CHANNELS_COUNT; ++i) {
        gpio_reset           (pwm_channels[i].gpio_port, pwm_channels[i].gpio_pin);
        gpio_set_mode        (pwm_channels[i].gpio_port, pwm_channels[i].gpio_pin, GPIO_MODE_OUTPUT);
        gpio_set_output_type (pwm_channels[i].gpio_port, pwm_channels[i].gpio_pin, GPIO_TYPE_PUSH_PULL);
        gpio_set_output_speed(pwm_channels[i].gpio_port, pwm_channels[i].gpio_pin, GPIO_SPEED_HIGH);
        gpio_set_pull        (pwm_channels[i].gpio_port, pwm_channels[i].gpio_pin, GPIO_PULL_NO);
        
        if (pwm_channels[i].gpio_port == GPIOA) pwm_gpio_a_bsr |= (0x01 << pwm_channels[i].gpio_pin);
        if (pwm_channels[i].gpio_port == GPIOB) pwm_gpio_b_bsr |= (0x01 << pwm_channels[i].gpio_pin);
        if (pwm_channels[i].gpio_port == GPIOC) pwm_gpio_c_bsr |= (0x01 << pwm_channels[i].gpio_pin);
        if (pwm_channels[i].gpio_port == GPIOD) pwm_gpio_d_bsr |= (0x01 << pwm_channels[i].gpio_pin);
        if (pwm_channels[i].gpio_port == GPIOE) pwm_gpio_e_bsr |= (0x01 << pwm_channels[i].gpio_pin);
    }

    // Setup PWM timer: one pulse mode
    RCC->APB2RSTR |= RCC_APB2RSTR_TIM17RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM17RST;
    TIM17->CR1  = TIM_CR1_OPM | TIM_CR1_URS; 
    TIM17->DIER = TIM_DIER_UIE;
    TIM17->PSC  = APB2_CLOCK_FREQUENCY / 1000000 - 1; // 1 tick = 1 us
    TIM17->ARR  = 1000000 / pwm_frequency; // Period
    NVIC_EnableIRQ(TIM17_IRQn);
    NVIC_SetPriority(TIM17_IRQn, TIM17_IRQ_PRIORITY);
}

/// ***************************************************************************
/// @brief  PWM enable/disable
/// @param  is_enable: true - PWM enable, false - PWM disable
/// ***************************************************************************
void pwm_set_state(bool is_enable) {
    if (is_enable) {
        TIM17->CR1 |= TIM_CR1_CEN;
    } else {
        TIM17->CR1 &= ~TIM_CR1_CEN;
    }
    
}

/// ***************************************************************************
/// @brief  PWM enable/disable
/// @param  is_enable: true - PWM enable, false - PWM disable
/// ***************************************************************************
void pwm_start_sync(void) {
    if (!pwm_sync_enabled) {
        pwm_sync_enabled = true;
        pwm_ready = false;
    }
}

/// ***************************************************************************
/// @brief  Set PWM frequency
/// @param  frequency: frequency [Hz]
/// ***************************************************************************
void pwm_set_frequency(uint32_t frequency) {
    if (frequency < PWM_MIN_FREQUENCY_HZ) {
        frequency = PWM_MIN_FREQUENCY_HZ;
    }
    if (frequency > PWM_MAX_FREQUENCY_HZ) {
        frequency = PWM_MAX_FREQUENCY_HZ;
    }
    pwm_frequency = frequency;
}

/// ***************************************************************************
/// @brief  Lock channels
/// @param  is_locked: true - buffer is lock, false - buffer is unlock
/// ***************************************************************************
void pwm_set_lock_state(bool is_locked) {
    if (!is_locked) {
        // Sorting PWM channels
        qsort(pwm_channels_ptr, SUPPORT_PWM_CHANNELS_COUNT, sizeof(pwm_channels_ptr[0]), compare_channels);
        
        // Apply ticks compensation (-1us by each 3 equals channels)
        for (uint32_t i = 0, equals = 0, prev_value = 0; i < SUPPORT_PWM_CHANNELS_COUNT; ++i) {
            if (prev_value == pwm_channels_ptr[i]->ticks) {
                ++equals;
            } else {
                prev_value = pwm_channels_ptr[i]->ticks;
                equals = 0;
            }
            pwm_channels_ptr[i]->ticks -= equals / 3;
        }
        
        // Sorting PWM channels
        qsort(pwm_channels_ptr, SUPPORT_PWM_CHANNELS_COUNT, sizeof(pwm_channels_ptr[0]), compare_channels);
    }
    pwm_locked = is_locked;
}

/// ***************************************************************************
/// @brief  Check PWM ready for load pulse width
/// @note   Setting each PWM period
/// ***************************************************************************
bool pwm_is_ready(void) {
    bool f = pwm_ready;
    pwm_ready = false;
    return f;
}

/// ***************************************************************************
/// @brief  Set PWM channel pulse width
/// @param  channel: PWM channel index
/// @param  width: pulse width
/// @return none
/// ***************************************************************************
void pwm_set_width(uint32_t channel, uint32_t width) {
    int32_t ticks = (int32_t)width;
    if (ticks < 0) {
        ticks = 0;
    }
    pwm_channels[channel].ticks = ticks;
}





/// ***************************************************************************
/// @brief  PWM timer ISR
/// ***************************************************************************
#pragma call_graph_root="interrupt"
void TIM17_IRQHandler(void) {
    if (TIM17->SR & TIM_SR_UIF) {  // Start next PWM period
        if (pwm_locked || pwm_ready) {
            sysmon_set_error(SYSMON_SYNC_ERROR);
        }
        
        // Reset state and update PWM frequency
        uint8_t ch_cursor = 0;
        TIM17->ARR = 1000000 / pwm_frequency;
        
        // Disable all interrupts
        uint32_t irq_state = __get_interrupt_state();
        __disable_interrupt();
        
        // Start PWM timer
        TIM17->SR = 0;
        TIM17->CR1 |= TIM_CR1_CEN;
        
        // Set HIGH level for PWM outputs
        GPIOA->BSRR = pwm_gpio_a_bsr;
        GPIOB->BSRR = pwm_gpio_b_bsr;
        GPIOC->BSRR = pwm_gpio_c_bsr;
        GPIOD->BSRR = pwm_gpio_d_bsr;
        GPIOE->BSRR = pwm_gpio_e_bsr;
        
        // Create pulse (HIGH)
        while (ch_cursor < SUPPORT_PWM_CHANNELS_COUNT) {
            if (pwm_channels_ptr[ch_cursor]->ticks > TIM17->ARR) {
                break; // D'not process unreached channels
            }
            while (TIM17->CNT < pwm_channels_ptr[ch_cursor]->ticks);
            gpio_reset(pwm_channels_ptr[ch_cursor]->gpio_port, pwm_channels_ptr[ch_cursor]->gpio_pin);
            ++ch_cursor;  
        }
        
        // Restore interrupts
        __set_interrupt_state(irq_state);
        
        pwm_ready = true;
    }
    TIM17->SR = 0;
}

/// ***************************************************************************
/// @brief  qsort compare function
/// ***************************************************************************
static int compare_channels(const void* a, const void* b) {
    pwm_channel_t* ch1 = *(pwm_channel_t**)a;
    pwm_channel_t* ch2 = *(pwm_channel_t**)b;
    if (ch1->ticks > ch2->ticks) return  1;
    if (ch1->ticks < ch2->ticks) return -1;
    return 0;
}
