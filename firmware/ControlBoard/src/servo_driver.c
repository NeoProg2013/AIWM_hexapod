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

#define SERVO_POWER_EN_PIN                      (2) // PC2
#define SERVO_TURN_POWER_ON()                   gpio_set  (GPIOC, SERVO_POWER_EN_PIN)
#define SERVO_TURN_POWER_OFF()                  gpio_reset(GPIOC, SERVO_POWER_EN_PIN)


#define SERVO_DS3218MG_270_ID                   (0x00)
#define       DS3218MG_MIN_PULSE_WIDTH          (500)
#define       DS3218MG_MAX_PULSE_WIDTH          (2500)
#define       DS3218MG_MAX_PHYSIC_ANGLE         (270)
#define       DS3218MG_LOGIC_ZERO               (DS3218MG_MAX_PHYSIC_ANGLE / 2)


typedef enum {
    OVERRIDE_LEVEL_NO,
    OVERRIDE_LEVEL_LOGIC_ANGLE,
    OVERRIDE_LEVEL_PHYSIC_ANGLE,
    OVERRIDE_LEVEL_PULSE_WIDTH
} override_level_t;

typedef struct {
    // Runtime information
    float logic_angle;
    float physic_angle;
    uint32_t pulse_width;
    override_level_t override_level;
    int32_t override_value;
    
    // EEPROM configuration
    uint8_t  config;
    int16_t  zero_trim;                             
    uint16_t logic_zero;
    uint16_t min_pulse_width;
    uint16_t max_pulse_width;
    uint16_t max_physic_angle;
} servo_t;


static servo_t servo_list[SUPPORT_SERVO_COUNT] = {0};


static bool read_configuration(void);
static float calculate_physic_angle(float logic_angle, const servo_t* servo);


//  ***************************************************************************
/// @brief  Servo driver initialization
/// @return none
//  ***************************************************************************
void servo_driver_init(void) {
    // Init servo power enable pin PC2): output mode, push-pull, high speed, no pull
    SERVO_TURN_POWER_OFF();
    gpio_set_mode        (GPIOC, SERVO_POWER_EN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_output_type (GPIOC, SERVO_POWER_EN_PIN, GPIO_TYPE_PUSH_PULL);
    gpio_set_output_speed(GPIOC, SERVO_POWER_EN_PIN, GPIO_SPEED_LOW);
    gpio_set_pull        (GPIOC, SERVO_POWER_EN_PIN, GPIO_PULL_NO);
   
    if (read_configuration() == false) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_SERVO_DRIVER);
        servo_driver_power_off();
        return;
    }
    
    pwm_init();
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
    
    // Set new logic angle
    servo_list[ch].logic_angle = angle;
}

//  ***************************************************************************
/// @brief  Servo driver power ON
//  ***************************************************************************
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
/// @brief  Servo driver process
/// @note   Call each PWM period
//  ***************************************************************************
void servo_driver_process(void) {
    if (sysmon_is_module_disable(SYSMON_MODULE_SERVO_DRIVER) == true) {
        servo_driver_power_off();
        return;
    }
    
    // Calculate servo state and update PWM driver
    pwm_set_shadow_buffer_lock_state(false);
    for (uint32_t i = 0; i < SUPPORT_SERVO_COUNT; ++i) {
        servo_t* servo = &servo_list[i];
            
        // Override logic angle if need
        if (servo->override_level == OVERRIDE_LEVEL_LOGIC_ANGLE) {
            servo->logic_angle = servo->override_value;
        }
        
        // Calculate physic angle
        servo->physic_angle = calculate_physic_angle(servo->logic_angle, servo);
        
        // Override physic angle if need
        if (servo->override_level == OVERRIDE_LEVEL_PHYSIC_ANGLE) {
            servo->physic_angle = servo->override_value;
        }

        // Calculate PWM pulse width  
        float step = (float)(servo->max_pulse_width - servo->min_pulse_width) / (float)servo->max_physic_angle;
        servo->pulse_width = (uint32_t)( (float)servo->min_pulse_width + servo->physic_angle * step );
    
        // Check pulse width value
        if (servo->pulse_width < servo->min_pulse_width || servo->pulse_width > servo->max_pulse_width) {
            sysmon_set_error(SYSMON_MATH_ERROR);
            sysmon_disable_module(SYSMON_MODULE_SERVO_DRIVER);
            servo_driver_power_off();
            return;
        }
        
        // Override pulse width if need
        if (servo->override_level == OVERRIDE_LEVEL_PULSE_WIDTH) {
            servo->pulse_width = servo->override_value;
        }
        
        // Load pulse width
        pwm_set_width(i, servo->pulse_width);
    }
    pwm_set_shadow_buffer_lock_state(false);
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
bool servo_driver_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response) {
    
    // Process commands without args
    if (strcmp(cmd, "power-off") == 0 && argc == 0) {
        servo_driver_power_off();
        return true;
    }
    else if (strcmp(cmd, "power-on") == 0 && argc == 0) {
        servo_driver_power_on();
        return true;
    }

    // Get servo index
    if (argc == 0) {
        strcpy(response, CLI_ERROR("Servo index is not found"));
        return false;
    }
    uint32_t servo_index = atoi(argv[0]);
    if (servo_index > SUPPORT_SERVO_COUNT) {
        strcpy(response, CLI_ERROR("Servo index is out of range"));
        return false;
    }
    
    //
    // Process command
    //
    servo_t* servo = &servo_list[servo_index];
    if (strcmp(cmd, "status") == 0 && argc == 1) {

        sprintf(response, CLI_OK("servo status report")
                          CLI_OK("    - override level: %ld")
                          CLI_OK("    - override value: %ld")
                          CLI_OK("    - logic angle: %ld")
                          CLI_OK("    - physic angle: %ld")
                          CLI_OK("    - pulse width: %lu"),
                servo->override_level, servo->override_value,
                (int32_t)servo->logic_angle, (int32_t)servo->physic_angle, servo->pulse_width);
    }
    else if (strcmp(cmd, "set-logic") == 0 && argc == 2) {
        servo->override_level = OVERRIDE_LEVEL_LOGIC_ANGLE;
        servo->override_value = atoi(argv[1]);
        sprintf(response, CLI_OK("[%lu] has new override level %lu"), servo_index, servo->override_level);
    }
    else if (strcmp(cmd, "set-physic") == 0 && argc == 2) {
        servo->override_level = OVERRIDE_LEVEL_LOGIC_ANGLE;
        servo->override_value = atoi(argv[1]);
        sprintf(response, CLI_OK("[%lu] has new override level %lu"), servo_index, servo->override_level);
    }
    else if (strcmp(cmd, "set-pulse") == 0 && argc == 2) {
        servo->override_level = OVERRIDE_LEVEL_LOGIC_ANGLE;
        servo->override_value = atoi(argv[1]);
        sprintf(response, CLI_OK("[%lu] has new override level %lu"), servo_index, servo->override_level);
    }
    else if (strcmp(cmd, "zero") == 0 && argc == 1) {
        servo->override_level = OVERRIDE_LEVEL_PHYSIC_ANGLE;
        servo->override_value = (uint32_t)calculate_physic_angle(0, servo);
        sprintf(response, CLI_OK("[%lu] moved to zero"), servo_index);
    }
    else if (strcmp(cmd, "reset") == 0 && argc == 1) {
        servo->override_level = OVERRIDE_LEVEL_NO;
        servo->override_value = 0;
        sprintf(response, CLI_OK("[%lu] has new override level %lu"), servo_index, servo->override_level);
    }
    else {
        strcpy(response, CLI_ERROR("Unknown command or format for servo driver"));
        return false;
    }
    return true;
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
        physic_angle = (float)servo->logic_zero - (servo->logic_angle + (float)servo->zero_trim);
    } else {
        physic_angle = (float)servo->logic_zero + (servo->logic_angle + (float)servo->zero_trim);
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
