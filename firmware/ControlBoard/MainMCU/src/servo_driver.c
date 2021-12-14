//  ***************************************************************************
/// @file    servo_driver.c
/// @author  NeoProg
//  ***************************************************************************
#include "servo_driver.h"
#include "project_base.h"
#include "configurator.h"
#include "cli.h"
#include "pwm.h"
#include "system_monitor.h"
#include "systimer.h"

#define SERVO_POWER_EN_PIN                      GPIOC, 2
#define SERVO_TURN_POWER_ON()                   gpio_set  (SERVO_POWER_EN_PIN)
#define SERVO_TURN_POWER_OFF()                  gpio_reset(SERVO_POWER_EN_PIN)


#define SERVO_DS3218MG_270_ID                   (0x00)
#define       DS3218MG_MIN_PULSE_WIDTH          (500)
#define       DS3218MG_MAX_PULSE_WIDTH          (2500)
#define       DS3218MG_MAX_PHYSIC_ANGLE         (270)
#define       DS3218MG_LOGIC_ZERO               (DS3218MG_MAX_PHYSIC_ANGLE / 2)


typedef enum {
    OVERRIDE_NO,
    OVERRIDE_LOGIC_ANGLE,
    OVERRIDE_PHYSIC_ANGLE,
    OVERRIDE_PULSE_WIDTH
} override_t;

typedef struct {
    // Runtime information
    float      logic_angle;
    float      physic_angle;
    uint16_t   pulse_width;
    override_t override_level;
    int16_t    override_value;
    
    // EEPROM configuration
    uint8_t  config;
    int16_t  zero_trim;                             
    uint16_t logic_zero;
    uint16_t min_pulse_width;
    uint16_t max_pulse_width;
    uint16_t max_physic_angle;
} servo_t;

CLI_CMD_HANDLER(servo_cli_cmd_help);
CLI_CMD_HANDLER(servo_cli_cmd_power);
CLI_CMD_HANDLER(servo_cli_cmd_logging);
CLI_CMD_HANDLER(servo_cli_cmd_calibration);
CLI_CMD_HANDLER(servo_cli_cmd_status);
CLI_CMD_HANDLER(servo_cli_cmd_set);
CLI_CMD_HANDLER(servo_cli_cmd_logic_zero);
CLI_CMD_HANDLER(servo_cli_cmd_physic_zero);
CLI_CMD_HANDLER(servo_cli_cmd_reset);

static const cli_cmd_t cli_cmd_list[] = {
    { .cmd = "help",        .handler = servo_cli_cmd_help             },
    { .cmd = "power",       .handler = servo_cli_cmd_power            },
    { .cmd = "logging",     .handler = servo_cli_cmd_logging          },
    { .cmd = "calibration", .handler = servo_cli_cmd_calibration      },
    { .cmd = "status",      .handler = servo_cli_cmd_status           },
    { .cmd = "set",         .handler = servo_cli_cmd_set              },
    { .cmd = "logic-zero",  .handler = servo_cli_cmd_logic_zero       },
    { .cmd = "physic-zero", .handler = servo_cli_cmd_physic_zero      },
    { .cmd = "reset",       .handler = servo_cli_cmd_reset            }
};


static servo_t servo_list[SUPPORT_SERVO_COUNT] = {0};
static bool is_enable_data_logging = false;


static bool read_configuration(void);
static float calculate_physic_angle(float logic_angle, const servo_t* servo);


//  ***************************************************************************
/// @brief  Servo driver initialization
/// @return none
//  ***************************************************************************
void servo_driver_init(void) {
    // Init servo power enable pin: output mode, push-pull, high speed, no pull
    SERVO_TURN_POWER_OFF();
    gpio_set_mode        (SERVO_POWER_EN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (SERVO_POWER_EN_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(SERVO_POWER_EN_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (SERVO_POWER_EN_PIN, GPIO_PULL_NO);
   
    if (read_configuration() == false) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_SERVO_DRIVER);
        servo_driver_power_off();
        return;
    }
    
    pwm_init(PWM_START_FREQUENCY_HZ);
}

//  ***************************************************************************
/// @brief  Start move servo to new angle
/// @param  ch:    servo channel
/// @param  angle: new angle
/// @return none
//  ***************************************************************************
void servo_driver_move(uint32_t ch, float angle) {
    if (ch >= SUPPORT_SERVO_COUNT) {
        sysmon_set_error(SYSMON_FATAL_ERROR);
        sysmon_disable_module(SYSMON_MODULE_SERVO_DRIVER);
        servo_driver_power_off();
        return;
    }
    servo_list[ch].logic_angle = angle;
}

//  ***************************************************************************
/// @brief  Servo driver power ON
//  *************+**************************************************************
void servo_driver_power_on(void) {
    SERVO_TURN_POWER_ON();
    pwm_enable();
}

//  ***************************************************************************
/// @brief  Servo driver power OFF
/// @note   Call before system reset
//  ***************************************************************************
void servo_driver_power_off(void) {
    SERVO_TURN_POWER_OFF();
    pwm_disable();
}

//  ***************************************************************************
/// @brief  Set servo speed
/// @param  speed: speed [0; 100]
//  ***************************************************************************
void servo_driver_set_speed(uint32_t speed) {
    speed = (speed > 100) ? 100 : speed;
    float factor = (float)speed / 100.0f;
    uint32_t freq = (uint32_t)(PWM_MIN_FREQUENCY_HZ + (float)(PWM_MAX_FREQUENCY_HZ - PWM_MIN_FREQUENCY_HZ) * factor);
    pwm_set_frequency(freq);
}

//  ***************************************************************************
/// @brief  Servo driver process
/// @note   Call each PWM period
//  ***************************************************************************
void servo_driver_process(void) {
    if (sysmon_is_module_disable(SYSMON_MODULE_SERVO_DRIVER) == true) {
        servo_driver_power_off();
        return;
    }
    
    // Calculate servo state and update PWM driver
    pwm_set_shadow_buffer_lock_state(true);
    for (uint32_t i = 0; i < SUPPORT_SERVO_COUNT; ++i) {
        servo_t* servo = &servo_list[i];
            
        // Override logic angle if need
        float logic_angle = servo->logic_angle;
        if (servo->override_level == OVERRIDE_LOGIC_ANGLE) {
            logic_angle = servo->override_value;
        }
        
        // Calculate physic angle
        servo->physic_angle = calculate_physic_angle(logic_angle, servo);
        
        // Override physic angle if need
        float physic_angle = servo->physic_angle;
        if (servo->override_level == OVERRIDE_PHYSIC_ANGLE) {
            physic_angle = servo->override_value;
        }

        // Calculate PWM pulse width  
        float step = (float)(servo->max_pulse_width - servo->min_pulse_width) / (float)servo->max_physic_angle;
        servo->pulse_width = (uint32_t)( (float)servo->min_pulse_width + physic_angle * step );
    
        // Check pulse width value
        if (servo->pulse_width < servo->min_pulse_width || servo->pulse_width > servo->max_pulse_width) {
            sysmon_set_error(SYSMON_MATH_ERROR);
            sysmon_disable_module(SYSMON_MODULE_SERVO_DRIVER);
            servo_driver_power_off();
            return;
        }
        
        // Override pulse width if need
        uint16_t pulse_width = servo->pulse_width;
        if (servo->override_level == OVERRIDE_PULSE_WIDTH) {
            pulse_width = servo->override_value;
        }
        
        // Load pulse width
        pwm_set_width(i, pulse_width);
    }
    pwm_set_shadow_buffer_lock_state(false);
    
    if (is_enable_data_logging) {
        void* tx_buffer = cli_get_tx_buffer();
        sprintf(tx_buffer, "[SERVO DRV]: %d %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", 
                (int32_t)get_time_ms(),
                (int32_t)servo_list[0].logic_angle, servo_list[0].pulse_width,
                (int32_t)servo_list[1].logic_angle, servo_list[1].pulse_width,
                (int32_t)servo_list[2].logic_angle, servo_list[2].pulse_width,
                (int32_t)servo_list[3].logic_angle, servo_list[3].pulse_width,
                (int32_t)servo_list[4].logic_angle, servo_list[4].pulse_width,
                (int32_t)servo_list[5].logic_angle, servo_list[5].pulse_width,
                (int32_t)servo_list[6].logic_angle, servo_list[6].pulse_width,
                (int32_t)servo_list[7].logic_angle, servo_list[7].pulse_width,
                (int32_t)servo_list[8].logic_angle, servo_list[8].pulse_width,
                (int32_t)servo_list[9].logic_angle, servo_list[9].pulse_width,
                (int32_t)servo_list[10].logic_angle, servo_list[10].pulse_width,
                (int32_t)servo_list[11].logic_angle, servo_list[11].pulse_width,
                (int32_t)servo_list[12].logic_angle, servo_list[12].pulse_width,
                (int32_t)servo_list[13].logic_angle, servo_list[13].pulse_width,
                (int32_t)servo_list[14].logic_angle, servo_list[14].pulse_width,
                (int32_t)servo_list[15].logic_angle, servo_list[15].pulse_width,
                (int32_t)servo_list[16].logic_angle, servo_list[16].pulse_width,
                (int32_t)servo_list[17].logic_angle, servo_list[17].pulse_width);
        cli_send_data(NULL);
    }
}

//  ***************************************************************************
/// @brief  Get command list for CLI
/// @param  cmd_list: pointer to cmd list size
/// @return command list
//  ***************************************************************************
const cli_cmd_t* servo_get_cmd_list(uint32_t* count) {
    *count = sizeof(cli_cmd_list) / sizeof(cli_cmd_t);
    return cli_cmd_list;
}





//  ***************************************************************************
/// @brief  Read configuration
/// @param  none
/// @return true - read success, false - fail
//  ***************************************************************************
static bool read_configuration(void) {
    for (uint32_t i = 0; i < SUPPORT_SERVO_COUNT; ++i) {
        servo_t* servo = &servo_list[i];
        
        // Calculate start address of servo configuration
        uint32_t base_address = MM_SERVO_CONFIG_BLOCK_BASE_EE_ADDRESS + i * MM_SERVO_CONFIG_BLOCK_SIZE;

        // Read servo configuration
        if (config_read_8(base_address + MM_SERVO_CONFIG_OFFSET, &servo->config) == false || servo->config == 0xFF) {
            return false;
        }
        
        // Load servo information
        uint8_t servo_id = (servo->config & MM_SERVO_CONFIG_SERVO_TYPE_MASK) >> 4;
        if (servo_id == SERVO_DS3218MG_270_ID) {
            servo->logic_zero       = DS3218MG_LOGIC_ZERO;
            servo->min_pulse_width  = DS3218MG_MIN_PULSE_WIDTH;
            servo->max_pulse_width  = DS3218MG_MAX_PULSE_WIDTH;
            servo->max_physic_angle = DS3218MG_MAX_PHYSIC_ANGLE;
        } else { // Unknown servo type
            return false;
        }

        // Read servo zero trim
        if (config_read_16(base_address + MM_SERVO_ZERO_TRIM_OFFSET, (uint16_t*)&servo->zero_trim) == false) {
            return false;
        }
    }
    return true;
}

//  ***************************************************************************
/// @brief  Calculate physic angle
/// @param  logic_angle: logic angle
/// @param  servo: servo object
/// @return physic angle value
//  ***************************************************************************
static float calculate_physic_angle(float logic_angle, const servo_t* servo) {
    
    // Convert logic angle to servo physic angle
    float physic_angle = 0;
    if (servo->config & MM_SERVO_CONFIG_REVERSE_DIRECTION_MASK) {
        physic_angle = servo->logic_zero - (logic_angle + (float)servo->zero_trim);
    } else {
        physic_angle = servo->logic_zero + (logic_angle + (float)servo->zero_trim);
    }
    
    // Check physic angle value
    if (physic_angle < 0 || physic_angle > servo->max_physic_angle) {
        sysmon_set_error(SYSMON_MATH_ERROR);
        sysmon_disable_module(SYSMON_MODULE_SERVO_DRIVER);
        servo_driver_power_off();
        return physic_angle;
    }
    return physic_angle;
}





// ***************************************************************************
// CLI SECTION
// ***************************************************************************
CLI_CMD_HANDLER(servo_cli_cmd_help) {
    const char* help = CLI_HELP(
        "[SERVO SUBSYSTEM]\r\n"
        "Commands: \r\n"
        "  servo power <0|1> - enable/disable servo power\r\n"
        "  servo logging <0|1> - enable/disable logging\r\n"
        "  servo calibration - move all servos to logic zero\r\n"
        "  servo status <servo idx> - print servo status\r\n"
        "  servo set <servo idx> <zero-trim|logic|physic|pulse> <value> - move servo\r\n"
        "  servo logic-zero <servo idx> - move servo to logic zero\r\n"
        "  servo physic-zero <servo idx> move servo to physic zero\r\n"
        "  servo reset <servo idx> return servo to subsystem control");
    if (strlen(response) >= USART1_TX_BUFFER_SIZE) {
        strcpy(response, CLI_ERROR("Help message size more USART1_TX_BUFFER_SIZE"));
        return false;
    }
    strcpy(response, help);
    return true;
}
CLI_CMD_HANDLER(servo_cli_cmd_power) {
    if (argc >= 1 && argv[0][0] == '1') {
        servo_driver_power_on();
    } else {
        servo_driver_power_off();
    }
    return true;
}
CLI_CMD_HANDLER(servo_cli_cmd_logging) {
    if (argc >= 1 && argv[0][0] == '1') {
        is_enable_data_logging = true;
    } else {
        is_enable_data_logging = false;
    }
    return true;
}
CLI_CMD_HANDLER(servo_cli_cmd_calibration) {
    for (uint32_t i = 0; i < SUPPORT_SERVO_COUNT; ++i) {
        servo_list[i].override_level = OVERRIDE_LOGIC_ANGLE;
        servo_list[i].override_value = 0;
    }
    sprintf(response, CLI_OK("moved all servo to logic zero"));
    return true;
}
CLI_CMD_HANDLER(servo_cli_cmd_status) {
    uint32_t servo_index = (argc == 1) ? atoi(argv[0]) : 0;
    servo_t* servo = &servo_list[(servo_index < SUPPORT_SERVO_COUNT) ? servo_index : SUPPORT_SERVO_COUNT - 1];
    sprintf(response, CLI_OK("servo status report")
                      CLI_OK("    - override level: %d")
                      CLI_OK("    - override value: %d")
                      CLI_OK("    - logic angle: %d")
                      CLI_OK("    - physic angle: %d")
                      CLI_OK("    - pulse width: %d")
                      CLI_OK("    - zero trim: %d"),
            servo->override_level, servo->override_value,
            (int32_t)servo->logic_angle, (int32_t)servo->physic_angle, servo->pulse_width, servo->zero_trim);
    return true;
}
CLI_CMD_HANDLER(servo_cli_cmd_set) {
    if (argc != 3) {
        strcpy(response, CLI_ERROR("Bad usage. Use \"servo help\" for details"));
        return false;
    }
    
    uint32_t servo_index = (atoi(argv[0]) < SUPPORT_SERVO_COUNT) ? atoi(argv[0]) : SUPPORT_SERVO_COUNT - 1;
    servo_t* servo = &servo_list[servo_index];
    
    if (strcmp(argv[1], "zero-trim") == 0) {
        servo->zero_trim = atoi(argv[2]);
        sprintf(response, CLI_OK("[%u] has new zero trim %d"), servo_index, servo->zero_trim);
    } 
    else if (strcmp(argv[1], "logic") == 0) {
        servo->override_level = OVERRIDE_LOGIC_ANGLE;
        servo->override_value = atoi(argv[2]);
        sprintf(response, CLI_OK("[%u] has new override level %d"), servo_index, servo->override_level);
    } 
    else if (strcmp(argv[1], "physic") == 0) {
        servo->override_level = OVERRIDE_PHYSIC_ANGLE;
        servo->override_value = atoi(argv[2]);
        sprintf(response, CLI_OK("[%u] has new override level %d"), servo_index, servo->override_level);
    } 
    else if (strcmp(argv[1], "pulse") == 0) {
        servo->override_level = OVERRIDE_PULSE_WIDTH;
        servo->override_value = atoi(argv[2]);
        sprintf(response, CLI_OK("[%u] has new override level %d"), servo_index, servo->override_level);
    } 
    else {
        strcpy(response, CLI_ERROR("Bad usage. Use \"servo help\" for details"));
        return false;
    }
    return true;
}
CLI_CMD_HANDLER(servo_cli_cmd_logic_zero) {
    uint32_t servo_index = (argc >= 1) ? atoi(argv[0]) : 0;
    servo_t* servo = &servo_list[(servo_index < SUPPORT_SERVO_COUNT) ? servo_index : SUPPORT_SERVO_COUNT - 1];
    servo->override_level = OVERRIDE_LOGIC_ANGLE;
    servo->override_value = 0;
    sprintf(response, CLI_OK("[%lu] moved to logic zero"), servo_index);
    return true;
}
CLI_CMD_HANDLER(servo_cli_cmd_physic_zero) {
    uint32_t servo_index = (argc >= 1) ? atoi(argv[0]) : 0;
    servo_t* servo = &servo_list[(servo_index < SUPPORT_SERVO_COUNT) ? servo_index : SUPPORT_SERVO_COUNT - 1];
    servo->override_level = OVERRIDE_PHYSIC_ANGLE;
    servo->override_value = (uint32_t)calculate_physic_angle(0, servo);
    sprintf(response, CLI_OK("[%lu] moved to physic zero"), servo_index);
    return true;
}
CLI_CMD_HANDLER(servo_cli_cmd_reset) {
    uint32_t servo_index = (argc >= 1) ? atoi(argv[0]) : 0;
    servo_t* servo = &servo_list[(servo_index < SUPPORT_SERVO_COUNT) ? servo_index : SUPPORT_SERVO_COUNT - 1];
    servo->override_level = OVERRIDE_NO;
    servo->override_value = 0;
    sprintf(response, CLI_OK("[%lu] has new override level %lu"), servo_index, servo->override_level);
    return true;
}
