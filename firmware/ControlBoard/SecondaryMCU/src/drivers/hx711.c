//  ***************************************************************************
/// @file    hx711.c
/// @author  NeoProg
//  ***************************************************************************
#include "hx711.h"
#include "systimer.h"
#include "stm32f0xx.h"
#include "project_base.h"

#define HX711_CALIB_SAMPLES     (20)
#define HX711_WAIT_TIMEOUT      (1000)    // ms
#define HX711_READ_TIMEOUT      (500)  // ms
#define HX711_MAX_ERRORS_COUNT  (10)

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
    gpio_set_pull(HX711_DI1_PIN, GPIO_PULL_NO);
    gpio_set_mode(HX711_DI2_PIN, GPIO_MODE_INPUT);
    gpio_set_pull(HX711_DI2_PIN, GPIO_PULL_NO);
    gpio_set_mode(HX711_DI3_PIN, GPIO_MODE_INPUT);
    gpio_set_pull(HX711_DI3_PIN, GPIO_PULL_NO);
    gpio_set_mode(HX711_DI4_PIN, GPIO_MODE_INPUT);
    gpio_set_pull(HX711_DI4_PIN, GPIO_PULL_NO);
    gpio_set_mode(HX711_DI5_PIN, GPIO_MODE_INPUT);
    gpio_set_pull(HX711_DI5_PIN, GPIO_PULL_NO);
    gpio_set_mode(HX711_DI6_PIN, GPIO_MODE_INPUT);
    gpio_set_pull(HX711_DI6_PIN, GPIO_PULL_NO);
    
    hx711_power_down();
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
    uint64_t start_wait = get_time_ms();
    bool is_ready = false;
    do {
        bool d1 = gpio_read(HX711_DI1_PIN) == false;
        bool d2 = gpio_read(HX711_DI2_PIN) == false;
        bool d3 = gpio_read(HX711_DI3_PIN) == false;
        bool d4 = gpio_read(HX711_DI4_PIN) == false;
        bool d5 = gpio_read(HX711_DI5_PIN) == false;
        bool d6 = gpio_read(HX711_DI6_PIN) == false;
        if (!d1 && !d2 && !d3 && !d4 && !d5 && !d6) {
            asm("NOP");
            break; // If no one device is not ready -- do not wait
        }
        is_ready = d1 && d2 && d3 && d4 && d5 && d6; 
    }
    while (!is_ready && get_time_ms() - start_wait < HX711_WAIT_TIMEOUT);
    
    // Check read timeout
    if (get_time_ms() - last_read_time > HX711_READ_TIMEOUT) {
        ++errors_count;
        hx711_power_down();
        hx711_power_up();
        return true; // After power UP select 128 gain & channel A
    }
    
    // Read data
    if (is_ready) {
        for (uint32_t i = 0; i < 6; ++i) {
            hx711_read_data[i] = 0;
        }
        
        for (uint32_t a = 0; a < 15; ++a) asm("NOP"); // Delay 2.5us
        
        for (int32_t i = 23; i >= 0; --i) {
            gpio_set(HX711_CLK_PIN);
            for (uint32_t a = 0; a < 15; ++a) asm("NOP"); // Delay 2.5us
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
        // No here delay -- change sign operation go on 5us
        
        // Change sign if need
        for (uint32_t i = 0; i < 6; ++i) {
            if (hx711_read_data[i] & 0x00800000) {
                hx711_read_data[i] |= 0xFF000000;
            }
            hx711_read_data[i] >>= 6; // Cut x LSB for reduce noise
        }
        
        // Falling pulse for 128 gain & channel A
        gpio_reset(HX711_CLK_PIN);
        for (uint32_t a = 0; a < 15; ++a) asm("NOP"); // Delay 2.5us
        
        is_data_updated = true;
        errors_count = 0;
        last_read_time = get_time_ms();
    }
    return true;
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
