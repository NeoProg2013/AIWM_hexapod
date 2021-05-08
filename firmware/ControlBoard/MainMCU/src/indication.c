//  ***************************************************************************
/// @file    indication.c
/// @author  NeoProg
//  ***************************************************************************
#include "indication.h"
#include "project_base.h"
#include "system_monitor.h"
#include "systimer.h"

#define LED_R_PIN                   GPIOC, 13
#define LED_G_PIN                   GPIOC, 14
#define LED_B_PIN                   GPIOC, 15
#define BUZZER_PIN                  GPIOC, 0

#define LED_TURN_OFF(pin)           gpio_reset(pin)
#define LED_TURN_ON(pin)            gpio_set  (pin)
#define BUZZER_TURN_OFF()           gpio_reset(BUZZER_PIN)
#define BUZZER_TURN_ON()            gpio_set  (BUZZER_PIN)


static bool is_cli_control_enabled = false;


static void blink_red_led_with_buzzer(uint32_t period);
static void blink_red_led(uint32_t period);
static void blink_blue_led(uint32_t period);
static void blink_yellow_led(uint32_t period);


//  ***************************************************************************
/// @brief  Driver initialization
/// @param  none
/// @return none
//  ***************************************************************************
void indication_init(void) {
    gpio_set_mode        (LED_R_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (LED_R_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(LED_R_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (LED_R_PIN, GPIO_PULL_NO);
    
    gpio_set_mode        (LED_G_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (LED_G_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(LED_G_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (LED_G_PIN, GPIO_PULL_NO);
    
    gpio_set_mode        (LED_B_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (LED_B_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(LED_B_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (LED_B_PIN, GPIO_PULL_NO);
    
    gpio_set_mode        (BUZZER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (BUZZER_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(BUZZER_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (BUZZER_PIN, GPIO_PULL_NO);

    
    // Disable all
    LED_TURN_OFF(LED_R_PIN);
    LED_TURN_OFF(LED_G_PIN);
    LED_TURN_OFF(LED_B_PIN);
    BUZZER_TURN_OFF();
    
    // Buzzer
    BUZZER_TURN_ON();
    delay_ms(80);
    BUZZER_TURN_OFF();
}

//  ***************************************************************************
/// @brief  Driver process
/// @param  none
/// @return none
//  ***************************************************************************
void indication_process(void) {
    if (is_cli_control_enabled) {
        return;
    }
    
    if (sysmon_is_error_set(SYSMON_ANY_ERROR) == false) {
        LED_TURN_OFF(LED_R_PIN);
        LED_TURN_ON(LED_G_PIN);
        LED_TURN_OFF(LED_B_PIN);
        BUZZER_TURN_OFF();
    } else {
        if (sysmon_is_error_set(SYSMON_VOLTAGE_ERROR) == true) {
             blink_red_led_with_buzzer(500);
        }
        else if (sysmon_is_error_set(SYSMON_FATAL_ERROR | SYSMON_CONFIG_ERROR | SYSMON_MEMORY_ERROR | SYSMON_MATH_ERROR) == true) {
             blink_red_led(100);
        }
        else if (sysmon_is_error_set(SYSMON_SYNC_ERROR | SYSMON_COMM_ERROR) == true) {
             blink_yellow_led(500);
        }
        else if (sysmon_is_error_set(SYSMON_CONN_LOST_ERROR) == true) {
             blink_blue_led(500);
        }
    }
}

//  ***************************************************************************
/// @brief  CLI command process
/// @param  cmd: command string
/// @param  argv: argument list
/// @param  argc: arguments count
/// @param  response: response
/// @retval response
/// @return true - success, false - fail
//  ***************************************************************************
bool indication_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response) {

    if (strcmp(cmd, "ext-ctrl") == 0 && argc == 1) {
        is_cli_control_enabled = atoi(argv[0]);
        LED_TURN_OFF(LED_R_PIN);
        LED_TURN_OFF(LED_G_PIN);
        LED_TURN_OFF(LED_B_PIN);
        BUZZER_TURN_OFF();
        if (is_cli_control_enabled) {
            strcpy(response, CLI_OK("Indication now is under control by CLI"));
        }
        else {
            strcpy(response, CLI_OK("Indication now is under control by MCU"));
        }
    }
    else if (strcmp(cmd, "set-state") == 0 && argc == 1) {
        char r = argv[0][0];
        char g = argv[0][1];
        char b = argv[0][2];
        char buzzer = argv[0][3];
        if (r != '0' && r != '1' || g != '0' && g != '1' || b != '0' && b != '1' || buzzer != '0' && buzzer != '1') {
            strcpy(response, CLI_ERROR("Wrong argument value. Possible 0 or 1"));
            return false;
        }
        
        if (r == '1') LED_TURN_ON(LED_R_PIN);
        else          LED_TURN_OFF(LED_R_PIN);
        if (g == '1') LED_TURN_ON(LED_G_PIN);
        else          LED_TURN_OFF(LED_G_PIN);
        if (b == '1') LED_TURN_ON(LED_B_PIN);
        else          LED_TURN_OFF(LED_B_PIN);
        if (buzzer == '1') BUZZER_TURN_ON();
        else               BUZZER_TURN_OFF();
    }
    else {
        strcpy(response, CLI_ERROR("Unknown command or format for indication"));
        return false;
    }
    return true;
}





//  ***************************************************************************
/// @brief  Blink red LED with buzzer beep
/// @param  period: blink and buzzer beep switch time
/// @return none
//  ***************************************************************************
static void blink_red_led_with_buzzer(uint32_t period) {
    
    static uint32_t start_time = 0;
    static bool state = false;
    
    if (get_time_ms() - start_time > period) {
        
        if (state == false) {
            LED_TURN_ON(LED_R_PIN);
            BUZZER_TURN_ON();
        }
        else {
            LED_TURN_OFF(LED_R_PIN);
            BUZZER_TURN_OFF();
        }
        LED_TURN_OFF(LED_G_PIN);
        LED_TURN_OFF(LED_B_PIN);
        
        state = !state;
        start_time = get_time_ms();
    }
}

//  ***************************************************************************
/// @brief  Blink red LED
/// @param  period: blink and buzzer beep switch time
/// @return none
//  ***************************************************************************
static void blink_red_led(uint32_t period) {
    
    static uint32_t start_time = 0;
    static bool state = false;
    
    if (get_time_ms() - start_time > period) {
        
        if (state == false) {
            LED_TURN_ON(LED_R_PIN);
        }
        else {
            LED_TURN_OFF(LED_R_PIN);
        }
        LED_TURN_OFF(LED_G_PIN);
        LED_TURN_OFF(LED_B_PIN);
        BUZZER_TURN_OFF();
        
        state = !state;
        start_time = get_time_ms();
    }
}

//  ***************************************************************************
/// @brief  Blink yellow LED
/// @param  period: LED switch time
/// @return none
//  ***************************************************************************
static void blink_yellow_led(uint32_t period) {
    
    static uint32_t start_time = 0;
    static bool state = false;
    
    if (get_time_ms() - start_time > period) {
        
        if (state == false) {
            LED_TURN_OFF(LED_R_PIN);
            LED_TURN_OFF(LED_G_PIN);
        }
        else {
            LED_TURN_ON(LED_R_PIN);
            LED_TURN_ON(LED_G_PIN);
        }
        LED_TURN_OFF(LED_B_PIN);
        BUZZER_TURN_OFF();
        
        state = !state;
        start_time = get_time_ms();
    }
}

//  ***************************************************************************
/// @brief  Blink blue LED
/// @param  period: LED switch time
/// @return none
//  ***************************************************************************
static void blink_blue_led(uint32_t period) {
    
    static uint32_t start_time = 0;
    static bool state = false;
    
    if (get_time_ms() - start_time > period) {
        
        if (state == false) {
            LED_TURN_OFF(LED_B_PIN);
        }
        else {
            LED_TURN_ON(LED_B_PIN);
        }
        LED_TURN_OFF(LED_R_PIN);
        LED_TURN_OFF(LED_G_PIN);
        BUZZER_TURN_OFF();
        
        state = !state;
        start_time = get_time_ms();
    }
}
