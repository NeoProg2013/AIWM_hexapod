//  ***************************************************************************
/// @file    systimer.c
/// @author  NeoProg
//  ***************************************************************************
#include "systimer.h"
#include "stm32f0xx.h"
#include "project_base.h"

#define HX711_WAIT_TIMEOUT      (5)    // ms
#define HX711_READ_TIMEOUT      (100)  // ms
#define HX711_MAX_ERRORS_COUNT  (10)

#define HX711_CLK_PIN           GPIOA, 0
#define HX711_DI1_PIN           GPIOB, 1
#define HX711_DI2_PIN           GPIOF, 1
#define HX711_DI3_PIN           GPIOF, 0
#define HX711_DI4_PIN           GPIOA, 7
#define HX711_DI5_PIN           GPIOA, 6
#define HX711_DI6_PIN           GPIOA, 5

static int32_t hx711_read_values[6] = {0};



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
    
    gpio_set_mode        (HX711_DI1_PIN, GPIO_MODE_INPUT);
    gpio_set_pull        (HX711_DI1_PIN, GPIO_PULL_NO);
    gpio_set_mode        (HX711_DI2_PIN, GPIO_MODE_INPUT);
    gpio_set_pull        (HX711_DI2_PIN, GPIO_PULL_NO);
    gpio_set_mode        (HX711_DI3_PIN, GPIO_MODE_INPUT);
    gpio_set_pull        (HX711_DI3_PIN, GPIO_PULL_NO);
    gpio_set_mode        (HX711_DI4_PIN, GPIO_MODE_INPUT);
    gpio_set_pull        (HX711_DI4_PIN, GPIO_PULL_NO);
    gpio_set_mode        (HX711_DI5_PIN, GPIO_MODE_INPUT);
    gpio_set_pull        (HX711_DI5_PIN, GPIO_PULL_NO);
    gpio_set_mode        (HX711_DI6_PIN, GPIO_MODE_INPUT);
    gpio_set_pull        (HX711_DI6_PIN, GPIO_PULL_NO);
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
/// @brief  HX711 process
/// @param  none
/// @return none
//  ***************************************************************************
bool hx711_process(int32_t* values) {
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
    uint64_t start_wait = get_time_ms();
    bool is_ready = false;
    do {
        bool d1 = gpio_read(HX711_DI1_PIN) == false;
        bool d2 = false;//gpio_read(HX711_DI2_PIN) == false;
        bool d3 = false;//gpio_read(HX711_DI3_PIN) == false;
        bool d4 = false;//gpio_read(HX711_DI4_PIN) == false;
        bool d5 = false;//gpio_read(HX711_DI5_PIN) == false;
        bool d6 = false;//gpio_read(HX711_DI6_PIN) == false;
        if (!(d1 || d2 || d3 || d4 || d5 || d6)) {
            break; // If no one device is not ready -- do not wait
        }
        is_ready = d1 && d2 && d3 && d4 && d5 && d6; 
    }
    while (!is_ready && get_time_ms() - start_wait > HX711_WAIT_TIMEOUT);
    
    // Check read timeout
    if (!is_ready && get_time_ms() - last_read_time > HX711_READ_TIMEOUT) {
        ++errors_count;
        hx711_power_down();
        hx711_power_up();
        return true; // After power UP select 128 gain & channel A
    }
    
    // Read data
    for (uint32_t i = 0; i < 6; ++i) {
        hx711_read_values[i] = 0;
    }
    
    for (int32_t i = 23; i >= 0; --i) {
        gpio_set(HX711_CLK_PIN);
        hx711_read_values[0] |= (int)gpio_read(HX711_DI1_PIN) << i;
        hx711_read_values[1] |= (int)gpio_read(HX711_DI2_PIN) << i;
        hx711_read_values[2] |= (int)gpio_read(HX711_DI3_PIN) << i;
        hx711_read_values[3] |= (int)gpio_read(HX711_DI4_PIN) << i;
        hx711_read_values[4] |= (int)gpio_read(HX711_DI5_PIN) << i;
        hx711_read_values[5] |= (int)gpio_read(HX711_DI6_PIN) << i;
        
        // Delay 25us
        for (uint32_t a = 0; a < 25; ++a) {
            asm("NOP");
        }
        
        gpio_reset(HX711_CLK_PIN);
        
        // Delay 25us
        for (uint32_t a = 0; a < 30; ++a) {
            asm("NOP");
        }
    }
    
    // Change sign if need
    for (uint32_t i = 0; i < 6; ++i) {
        hx711_read_values[i] ^= 0x00800000;
        hx711_read_values[i] -= 0x007FFFFF;
    }
    
    // Send pulse for 128 gain & channel A
    gpio_set(HX711_CLK_PIN);
    for (uint32_t a = 0; a < 30; ++a) {
        asm("NOP");
    }
    gpio_reset(HX711_CLK_PIN);
    for (uint32_t a = 0; a < 30; ++a) {
        asm("NOP");
    }
    
    last_read_time = get_time_ms();
    return true;
}

//  ***************************************************************************
/// @brief  HX711 read values
/// @param  values: pointer to buffer for data
/// @return none
//  ***************************************************************************
void hx711_read(int32_t* buffer) {
    for (uint32_t i = 0; i < 6; ++i) {
        buffer[i] = hx711_read_values[i];
    }
}
