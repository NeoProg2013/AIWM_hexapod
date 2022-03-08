/// ***************************************************************************
/// @file    indication.c
/// @author  NeoProg
/// ***************************************************************************
#include "indication.h"
#include "project-base.h"
#include "system-monitor.h"
#include "systimer.h"

#define LED_R_PIN                   GPIOC, 13 // PC13
#define LED_G_PIN                   GPIOC, 14 // PC14
#define LED_B_PIN                   GPIOC, 15 // PC15

#define ACTIVE_LED_PIN              GPIOA, 15 // PA15


static bool is_external_control = false;


static void blink_red_led(uint32_t period);
static void blink_green_led(uint32_t period);
static void blink_blue_led(uint32_t period);
static void blink_yellow_led(uint32_t period);
static void blink_red_yellow_led(uint32_t period);
static void blink_active_led(void);

CLI_CMD_HANDLER(indication_cli_cmd_help);
CLI_CMD_HANDLER(indication_cli_cmd_ext_ctrl);
CLI_CMD_HANDLER(indication_cli_cmd_set);

static const cli_cmd_t cli_cmd_list[] = {
    { .cmd = "help",      .handler = indication_cli_cmd_help     },
    { .cmd = "ext-ctrl",  .handler = indication_cli_cmd_ext_ctrl },
    { .cmd = "set",       .handler = indication_cli_cmd_set      }
};


/// ***************************************************************************
/// @brief  Driver initialization
/// @param  none
/// @return none
/// ***************************************************************************
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
    
    gpio_reset           (ACTIVE_LED_PIN);
    gpio_set_mode        (ACTIVE_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (ACTIVE_LED_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(ACTIVE_LED_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (ACTIVE_LED_PIN, GPIO_PULL_NO);

    
    // Disable all
    gpio_reset(LED_R_PIN);
    gpio_reset(LED_G_PIN);
    gpio_reset(LED_B_PIN);
    gpio_reset(ACTIVE_LED_PIN);
}

/// ***************************************************************************
/// @brief  Driver process
/// @param  none
/// @return none
/// ***************************************************************************
void indication_process(void) {
    blink_active_led();
    if (is_external_control) return;
    
    if (!sysmon_is_error_set(SYSMON_ANY_ERROR)) {
        gpio_reset(LED_R_PIN);
        gpio_set(LED_G_PIN);
        gpio_reset(LED_B_PIN);
    } else {
        if (sysmon_is_error_set(SYSMON_FATAL_ERROR)) {
            blink_red_led(100);
            return;
        }
        if (sysmon_is_error_set(SYSMON_CONN_LOST_ERROR)) {
            if (sysmon_is_error_set(SYSMON_VOLTAGE_ERROR)) {
                blink_red_led(500);
            } else if (sysmon_is_error_set(SYSMON_I2C_ERROR)) {
                blink_yellow_led(500);
            } else if (sysmon_is_error_set(SYSMON_SYNC_ERROR)) {
                blink_green_led(500);
            } else if (sysmon_is_error_set(SYSMON_MATH_ERROR)) {
                blink_red_yellow_led(500);
            } else {
                blink_blue_led(500);
            }
        } else {
            if (sysmon_is_error_set(SYSMON_VOLTAGE_ERROR)) {
                gpio_set(LED_R_PIN);
                gpio_reset(LED_G_PIN);
                gpio_reset(LED_B_PIN);
            } else if (sysmon_is_error_set(SYSMON_I2C_ERROR)) {
                gpio_set(LED_R_PIN);
                gpio_set(LED_G_PIN);
                gpio_reset(LED_B_PIN);
            } else if (sysmon_is_error_set(SYSMON_SYNC_ERROR)) {
                gpio_reset(LED_R_PIN);
                gpio_set(LED_G_PIN);
                gpio_reset(LED_B_PIN);
            } else if (sysmon_is_error_set(SYSMON_MATH_ERROR)) {
                blink_red_yellow_led(500);
            }
        }
    }
}

/// ***************************************************************************
/// @brief  Get command list for CLI
/// @param  cmd_list: pointer to cmd list size
/// @return command list
/// ***************************************************************************
const cli_cmd_t* indication_get_cmd_list(uint32_t* count) {
    *count = sizeof(cli_cmd_list) / sizeof(cli_cmd_t);
    return cli_cmd_list;
}





/// ***************************************************************************
/// @brief  Blink red LED
/// @param  period: LED switch time
/// ***************************************************************************
static void blink_red_led(uint32_t period) {
    static uint32_t start_time = 0;
    static bool state = false;
    
    if (get_time_ms() - start_time > period) {
        if (state == false) {
            gpio_set(LED_R_PIN);
        } else {
            gpio_reset(LED_R_PIN);
        }
        gpio_reset(LED_G_PIN);
        gpio_reset(LED_B_PIN);
        
        state = !state;
        start_time = get_time_ms();
    }
}

/// ***************************************************************************
/// @brief  Blink green LED
/// @param  period: LED switch time
/// ***************************************************************************
static void blink_green_led(uint32_t period) {
    static uint32_t start_time = 0;
    static bool state = false;
    
    if (get_time_ms() - start_time > period) {
        if (state == false) {
            gpio_reset(LED_G_PIN);
        } else {
            gpio_set(LED_G_PIN);
        }
        gpio_set(LED_R_PIN);
        gpio_reset(LED_B_PIN);
        
        state = !state;
        start_time = get_time_ms();
    }
}

/// ***************************************************************************
/// @brief  Blink yellow LED
/// @param  period: LED switch time
/// @return none
/// ***************************************************************************
static void blink_yellow_led(uint32_t period) {
    static uint32_t start_time = 0;
    static bool state = false;
    
    if (get_time_ms() - start_time > period) {
        if (state == false) {
            gpio_reset(LED_R_PIN);
            gpio_reset(LED_G_PIN);
        } else {
            gpio_set(LED_R_PIN);
            gpio_set(LED_G_PIN);
        }
        gpio_reset(LED_B_PIN);
        
        state = !state;
        start_time = get_time_ms();
    }
}

/// ***************************************************************************
/// @brief  Blink blue LED
/// @param  period: LED switch time
/// ***************************************************************************
static void blink_blue_led(uint32_t period) {
    static uint32_t start_time = 0;
    static bool state = false;
    
    if (get_time_ms() - start_time > period) {
        if (state == false) {
            gpio_reset(LED_B_PIN);
        } else {
            gpio_set(LED_B_PIN);
        }
        gpio_reset(LED_R_PIN);
        gpio_reset(LED_G_PIN);
        
        state = !state;
        start_time = get_time_ms();
    }
}

/// ***************************************************************************
/// @brief  Blink red & yellow LEDs
/// @param  period: LED switch time
/// ***************************************************************************
static void blink_red_yellow_led(uint32_t period) {
    static uint32_t start_time = 0;
    static bool state = false;
    
    if (get_time_ms() - start_time > period) {
        if (state == false) {
            gpio_reset(LED_G_PIN);
            gpio_set(LED_R_PIN);
        } else {
            gpio_set(LED_G_PIN);
            gpio_set(LED_R_PIN);
        }
        gpio_reset(LED_B_PIN);
        
        state = !state;
        start_time = get_time_ms();
    }
}

/// ***************************************************************************
/// @brief  Blink onboard active LED
/// ***************************************************************************
static void blink_active_led(void) {
    static uint64_t last_time = 0;
    if (get_time_ms() - last_time > 1000) {
        gpio_toggle(ACTIVE_LED_PIN);
        last_time = get_time_ms();
    }
}





// ***************************************************************************
// CLI SECTION
// ***************************************************************************
CLI_CMD_HANDLER(indication_cli_cmd_help) {
    const char* help = CLI_HELP(
        "[INDICATION SUBSYSTEM]\r\n"
        "Commands: \r\n"
        "  indication ext-ctrl <0|1> - enable subsystem after error\r\n"
        "  indication set [0|1] [0|1] [0|1] - set state for RGB");
    if (strlen(response) >= USART1_TX_BUFFER_SIZE) {
        strcpy(response, CLI_ERROR("Help message size more USART1_TX_BUFFER_SIZE"));
        return false;
    }
    strcpy(response, help);
    return true;
}
CLI_CMD_HANDLER(indication_cli_cmd_ext_ctrl) {
    if (argc != 1) {
        strcpy(response, CLI_ERROR("Bad usage. Usage \"indication help\" for details"));
        return false;
    }
    is_external_control = atoi(argv[0]);
    gpio_reset(LED_R_PIN);
    gpio_reset(LED_G_PIN);
    gpio_reset(LED_B_PIN);
    if (is_external_control) {
        strcpy(response, CLI_OK("Indication now is under control by CLI"));
    } else {
        strcpy(response, CLI_OK("Indication now is under control by MCU"));
    }
    return true;
}
CLI_CMD_HANDLER(indication_cli_cmd_set) {
    char r = (argc >= 1) ? argv[0][0] : '0';
    char g = (argc >= 2) ? argv[1][0] : '0';
    char b = (argc >= 3) ? argv[2][0] : '0';
    if (r != '0' && r != '1' || g != '0' && g != '1' || b != '0' && b != '1') {
        strcpy(response, CLI_ERROR("Bad usage. Usage \"indication help\" for details"));
        return false;
    }
    
    if (r == '1') gpio_set(LED_R_PIN);
    else          gpio_reset(LED_R_PIN);
    if (g == '1') gpio_set(LED_G_PIN);
    else          gpio_reset(LED_G_PIN);
    if (b == '1') gpio_set(LED_B_PIN);
    else          gpio_reset(LED_B_PIN);
    
    return true;
}
