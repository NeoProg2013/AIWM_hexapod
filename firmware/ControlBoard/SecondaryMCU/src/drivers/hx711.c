//  ***************************************************************************
/// @file    hx711.c
/// @author  NeoProg
//  ***************************************************************************
#include "hx711.h"
#include "systimer.h"
#include "stm32f0xx.h"
#include "project_base.h"

#define HX711_CALIB_SAMPLES     (100)
#define HX711_WAIT_TIMEOUT      (50)   // ms
#define HX711_READ_TIMEOUT      (200)  // ms
#define HX711_MAX_ERRORS_COUNT  (10)

#define HX711_SYNC_PIN          GPIOA, 4
#define HX711_CLK_PIN           GPIOA, 0
#define HX711_DI1_PIN           GPIOB, 1
#define HX711_DI2_PIN           GPIOF, 1
#define HX711_DI3_PIN           GPIOF, 0
#define HX711_DI4_PIN           GPIOA, 7
#define HX711_DI5_PIN           GPIOA, 6
#define HX711_DI6_PIN           GPIOA, 5

static int32_t hx711_read_data[6] = {0};
static int32_t hx711_offset[6] = {0};
static bool is_data_updated = false;
static bool is_any_data_ready = false;


//  ***************************************************************************
/// @brief  HX711 driver initialization
/// @param  none
/// @return none
//  ***************************************************************************
void hx711_init(void) {
    gpio_set             (HX711_CLK_PIN);
    gpio_set_mode        (HX711_CLK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (HX711_CLK_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(HX711_CLK_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (HX711_CLK_PIN, GPIO_PULL_NO);
    
    gpio_set_mode(HX711_DI1_PIN, GPIO_MODE_INPUT);
    gpio_set_pull(HX711_DI1_PIN, GPIO_PULL_UP);
    gpio_set_mode(HX711_DI2_PIN, GPIO_MODE_INPUT);
    gpio_set_pull(HX711_DI2_PIN, GPIO_PULL_UP);
    gpio_set_mode(HX711_DI3_PIN, GPIO_MODE_INPUT);
    gpio_set_pull(HX711_DI3_PIN, GPIO_PULL_UP);
    gpio_set_mode(HX711_DI4_PIN, GPIO_MODE_INPUT);
    gpio_set_pull(HX711_DI4_PIN, GPIO_PULL_UP);
    gpio_set_mode(HX711_DI5_PIN, GPIO_MODE_INPUT);
    gpio_set_pull(HX711_DI5_PIN, GPIO_PULL_UP);
    gpio_set_mode(HX711_DI6_PIN, GPIO_MODE_INPUT);
    gpio_set_pull(HX711_DI6_PIN, GPIO_PULL_UP);
    
    hx711_power_down();
    
    // Setup GPIO interrupt
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PF;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PF;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PF;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PF;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI7_PF;
    EXTI->IMR |= EXTI_IMR_MR0 | EXTI_IMR_MR1 | EXTI_IMR_MR5 | EXTI_IMR_MR6 | EXTI_IMR_MR7;
    EXTI->FTSR |= EXTI_FTSR_TR0 | EXTI_FTSR_TR1 | EXTI_FTSR_TR5 | EXTI_FTSR_TR6 | EXTI_FTSR_TR7;
    NVIC_EnableIRQ(EXTI0_1_IRQn);
    NVIC_EnableIRQ(EXTI2_3_IRQn);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
    
    // Setup synchronize clock source
    gpio_reset           (HX711_SYNC_PIN);
    gpio_set_mode        (HX711_SYNC_PIN, GPIO_MODE_AF);
    gpio_set_output_speed(HX711_SYNC_PIN, GPIO_SPEED_HIGH);
    gpio_set_af          (HX711_SYNC_PIN, 4);
    
    TIM14->PSC    = 0x0000;
    TIM14->CCR1   = 1;
    TIM14->ARR    = TIM14->CCR1 * 2;
    TIM14->CCMR1 |= (0x06 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1FE;
    TIM14->CCER  |= TIM_CCER_CC1E;
    TIM14->CR1   |= TIM_CR1_CEN;
}

//  ***************************************************************************
/// @brief  HX711 power up
/// @param  none
/// @return none
//  ***************************************************************************
void hx711_power_up(void) {
    gpio_reset(HX711_CLK_PIN);
    delay_ms(1);
}

//  ***************************************************************************
/// @brief  HX711 power down
/// @param  none
/// @return none
//  ***************************************************************************
void hx711_power_down(void) {
    gpio_set(HX711_CLK_PIN);
    delay_ms(1);
}

//  ***************************************************************************
/// @brief  HX711 calibration
/// @param  none
/// @return none
//  ***************************************************************************
bool hx711_calibration(void) {
    for (uint32_t i = 0; i < 50; ++i) { // Skip few first samples after power up
        while (!is_data_updated) {
            if (!hx711_process()) {
                return false;
            }
        }
        int32_t data[6] = {0};
        hx711_read(data);
    }
    
    // Reset offsets
    for (uint32_t i = 0; i < 6; ++i) {
        hx711_offset[i] = 0;
    }
    
    // Read samples for calcluate offsets
    int32_t acc[6] = {0};
    for (uint32_t i = 0; i < HX711_CALIB_SAMPLES; ++i) {
        while (!is_data_updated) {
            if (!hx711_process()) {
                return false;
            }
        }
        int32_t data[6] = {0};
        hx711_read(data);
        
        for (uint32_t a = 0; a < 6; ++a) {
            acc[a] += data[a];
        }
    }
    
    // Calculate offsets
    for (uint32_t i = 0; i < 6; ++i) {
        hx711_offset[i] = acc[i] / HX711_CALIB_SAMPLES;
    }
    return true;
}

//  ***************************************************************************
/// @brief  HX711 process
/// @param  none
/// @return none
//  ***************************************************************************
bool hx711_process(void) {
    static uint64_t last_read_time = (uint64_t)(-1);
    static uint32_t errors_count = 0;
    
    // Initialization variable for first call
    if (last_read_time == (uint64_t)(-1)) {
        last_read_time = get_time_ms();
    }
    
    // Disable driver if communication lost
    if (errors_count >= HX711_MAX_ERRORS_COUNT) {
        hx711_power_down();
        return false;
    }
    
    // Wait LOW level on data pins
    bool is_all_data_ready = false;
    if (is_any_data_ready) {
        uint64_t start_wait = get_time_ms();
        do {
            bool d1 = gpio_read(HX711_DI1_PIN) == false;
            bool d2 = gpio_read(HX711_DI2_PIN) == false;
            bool d3 = gpio_read(HX711_DI3_PIN) == false;
            bool d4 = gpio_read(HX711_DI4_PIN) == false;
            bool d5 = gpio_read(HX711_DI5_PIN) == false;
            bool d6 = gpio_read(HX711_DI6_PIN) == false;
            is_all_data_ready = d1 && d2 && d3 && d4 && d5 && d6; 
        }
        while (!is_all_data_ready && get_time_ms() - start_wait < HX711_WAIT_TIMEOUT);
    }
    is_any_data_ready = false;
    
    // Read data
    if (is_all_data_ready) {
        for (uint32_t i = 0; i < 6; ++i) {
            hx711_read_data[i] = 0;
        }
        
        for (uint32_t a = 0; a < 15; ++a) asm("NOP"); // Delay 2.5us (for 48MHz clocks)
        
        for (int32_t i = 23; i >= 0; --i) {
            gpio_set(HX711_CLK_PIN);
            for (uint32_t a = 0; a < 15; ++a) asm("NOP"); // Delay 2.5us (for 48MHz clocks)
            gpio_reset(HX711_CLK_PIN);
            // No here delay -- read operation go on 5us
            
            hx711_read_data[0] |= (int)gpio_read(HX711_DI1_PIN) << i;
            hx711_read_data[1] |= (int)gpio_read(HX711_DI2_PIN) << i;
            hx711_read_data[2] |= (int)gpio_read(HX711_DI3_PIN) << i;
            hx711_read_data[3] |= (int)gpio_read(HX711_DI4_PIN) << i;
            hx711_read_data[4] |= (int)gpio_read(HX711_DI5_PIN) << i;
            hx711_read_data[5] |= (int)gpio_read(HX711_DI6_PIN) << i;
        }
        
        // Rising pulse for 128 gain & channel A
        gpio_set(HX711_CLK_PIN);
        // No here delay -- change sign operation go on 5us (for 48MHz clocks)
        
        // Change sign if need
        for (uint32_t i = 0; i < 6; ++i) {
            if (hx711_read_data[i] & 0x00800000) {
                hx711_read_data[i] |= 0xFF000000;
            }
            hx711_read_data[i] >>= 6; // Cut x LSB for reduce noise
        }
        
        // Falling pulse for 128 gain & channel A
        gpio_reset(HX711_CLK_PIN);
        for (uint32_t a = 0; a < 15; ++a) asm("NOP"); // Delay 2.5us (for 48MHz clocks)
        
        is_data_updated = true;
        errors_count = 0;
        last_read_time = get_time_ms();
    }
    
    // Check read timeout
    if (get_time_ms() - last_read_time > HX711_READ_TIMEOUT) {
        ++errors_count;
        hx711_power_down();
        hx711_power_up();
        // After power UP select 128 gain & channel A
    }
    return true;
}

//  ***************************************************************************
/// @brief  Process DI GPIO interrupt
/// @param  pr: EXTI->PR register value
/// @return none
//  ***************************************************************************
void hx711_process_irq(uint32_t pr) {
    if (pr & 0x00000073) {
        is_any_data_ready = true;
    }
}

//  ***************************************************************************
/// @brief  HX711 read values
/// @param  values: pointer to buffer for data
/// @return none
//  ***************************************************************************
bool hx711_read(int32_t* buffer) {
    bool prev_flag_value = is_data_updated;
    for (uint32_t i = 0; i < 6; ++i) {
        buffer[i] = hx711_read_data[i] - hx711_offset[i];
    }
    is_data_updated = false;
    return prev_flag_value;
}
