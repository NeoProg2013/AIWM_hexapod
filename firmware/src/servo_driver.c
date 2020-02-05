//  ***************************************************************************
/// @file    servo_driver.c
/// @author  NeoProg
//  ***************************************************************************
#include "servo_driver.h"
#include "memory_map.h"
#include "configurator.h"
#include "cli.h"
#include "pwm.h"
#include "system_monitor.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVO_DS3218MG_270_ID                   (0x00)
#define       DS3218MG_MIN_PULSE_WIDTH          (500)
#define       DS3218MG_MAX_PULSE_WIDTH          (2500)
#define       DS3218MG_MAX_PHYSIC_ANGLE         (270)


typedef struct {
    uint8_t  config;                            // Servo mode configuration
    uint8_t  pwm_channel;                       // PWM channel
    uint16_t logic_zero;                        // Servo logic zero
    uint16_t protection_min_physic_angle;       // Protection min servo physic angle, [degree]
    uint16_t protection_max_physic_angle;       // Protection max servo physic angle, [degree]

    uint16_t min_pulse_width;                   // Min pulse width
    uint16_t max_pulse_width;                   // Max pulse width
    uint16_t max_physic_angle;                  // Max physic angle
} servo_config_t;

typedef struct {
	float logic_angle;
	float physic_angle;
	uint32_t pulse_width;
} servo_info_t;


static servo_config_t servo_config_list[SUPPORT_SERVO_COUNT] = { 0 };
static servo_info_t   servo_info_list[SUPPORT_SERVO_COUNT] = { 0 };

uint32_t override_control = 0;
servo_info_t servo_info_override_list[SUPPORT_SERVO_COUNT] = { 0 };


static bool read_configuration(void);
static float calculate_physic_angle(float logic_angle, const servo_config_t* config);
static uint32_t convert_angle_to_pulse_width(float physic_angle, const servo_config_t* config);


//  ***************************************************************************
/// @brief  Servo driver initialization
/// @return none
//  ***************************************************************************
void servo_driver_init(void) {

    if (read_configuration() == false) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_SERVO_DRIVER);
        return;
    }

    pwm_init();
    pwm_enable();

    // Move servos to zero position
    for (uint32_t i = 0; i < SUPPORT_SERVO_COUNT; ++i) {
        servo_driver_move(i, 0);
    }
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
        return;
    }

    servo_info_list[ch].logic_angle = angle;
}

//  ***************************************************************************
/// @brief  Servo driver process
/// @note   Call from main loop
//  ***************************************************************************
void servo_driver_process(void) {

    if (sysmon_is_module_disable(SYSMON_MODULE_SERVO_DRIVER) == true) {
        pwm_disable();
        return;
    }
	
	static uint64_t prev_synchro = 0;
	if (synchro != prev_synchro) {

		// We skip PWM period - this is error and calculations have long time
		if (synchro - prev_synchro > 1) {
			sysmon_set_error(SYSMON_SYNC_ERROR);
            sysmon_disable_module(SYSMON_MODULE_SERVO_DRIVER);
		}

		//
		// Calculate servo state and update PWM driver
		//
		pwm_lock_shadow_buffer(false);
		for (uint32_t i = 0; i < SUPPORT_SERVO_COUNT; ++i) {

			servo_info_t* info = &servo_info_list[i];
			servo_info_t* info_override = &servo_info_override_list[i];

			// Override logic angle if need
			if (override_control & (1 << i)) {
				info->logic_angle = info_override->logic_angle;
			}

			// Calculate physic angle
			info->physic_angle = calculate_physic_angle(info->logic_angle, &servo_config_list[i]);

			// Override physic angle if need
			if (override_control & (1 << i)) {
				info->physic_angle = info_override->physic_angle;
			}

			// Calculate pulse width
			info->pulse_width = convert_angle_to_pulse_width(info->physic_angle, &servo_config_list[i]);

			// Override pulse width if need
			if (override_control & (1 << i)) {
				info->pulse_width = info_override->pulse_width;
			}

			// Load pulse width
			pwm_set_width(servo_config_list[i].pwm_channel, info->pulse_width);
		}
		pwm_lock_shadow_buffer(false);

		prev_synchro = synchro;
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
bool servo_driver_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response) {

	if (strcmp(cmd, "enable") == 0) {
    	sysmon_enable_module(SYSMON_MODULE_SERVO_DRIVER);
	}
	else if (strcmp(cmd, "disable") == 0) {
    	sysmon_disable_module(SYSMON_MODULE_SERVO_DRIVER);
	}
    else if (strcmp(cmd, "detach") == 0) {

		if (argc != 1) return false;

		// Get servo index
		uint32_t servo_index = atoi(argv[0]);
		if (servo_index > SUPPORT_SERVO_COUNT) return false;

		// Detach servo from driver control
		override_control |= (1 << servo_index);
		servo_info_override_list[servo_index] = servo_info_list[servo_index];
        
        // Make response
        sprintf(response, CLI_MSG("servo [%lu] detached from driver"), servo_index);
	}
	else if (strcmp(cmd, "attach") == 0) {

		if (argc != 1) return false;

		// Get servo index
		uint32_t servo_index = atoi(argv[0]);
		if (servo_index > SUPPORT_SERVO_COUNT) return false;

		// Attach servo to driver control
		override_control &= ~(1 << servo_index);
        
        // Make response
        sprintf(response, CLI_MSG("servo [%lu] attached to driver"), servo_index);
	}
	else if (strcmp(cmd, "status") == 0) {

		if (argc != 1) return false;

		// Get servo index
		uint32_t servo_index = atoi(argv[0]);
		if (servo_index > SUPPORT_SERVO_COUNT) return false;
		
		servo_info_t* info = &servo_info_list[servo_index];

        // Make response
		sprintf(response, CLI_MSG("servo status report")
		                  CLI_MSG("    - control status: %s")
		                  CLI_MSG("    - logic_angle: %ld")
		                  CLI_MSG("    - physic_angle: %ld")
		                  CLI_MSG("    - pulse_width: %lu"),
                (override_control & (1 << servo_index)) ? "user" : "driver",
				(int32_t)info->logic_angle, (int32_t)info->physic_angle, info->pulse_width);
	}
	else if (strcmp(cmd, "set_angle") == 0) {

		if (argc != 3) return false;
		
		// Get servo index
		uint32_t servo_index = atoi(argv[0]);
		if (servo_index > SUPPORT_SERVO_COUNT) return false;

		// Get angle
		float angle = atof(argv[2]);
		
		// Get angle type
		if (strcmp(argv[1], "log") == 0) {
			servo_info_override_list[servo_index].logic_angle = angle;
			sprintf(response, CLI_MSG("servo [%lu] have new logic angle (rouded) [%ld]"), servo_index, (int32_t)angle);
		}
		else if (strcmp(argv[1], "phy") == 0) {
			servo_info_override_list[servo_index].physic_angle = angle;
			sprintf(response, CLI_MSG("servo [%lu] have new physic angle (rouded) [%ld]"), servo_index, (int32_t)angle);
		}
		else {
			return false;
		}
	}
	else if (strcmp(cmd, "set_pulse") == 0) {

		if (argc != 2) return false;

		// Get servo index
		uint32_t servo_index = atoi(argv[0]);
		if (servo_index > SUPPORT_SERVO_COUNT) return false;

		// Get angle
		uint32_t pulse_width = atoi(argv[1]);

		// Set new servo angle
		servo_info_override_list[servo_index].pulse_width = pulse_width;
        
        // Make response
        sprintf(response, CLI_MSG("servo [%lu] have new pulse width [%lu]"), servo_index, pulse_width);
	}
	else {
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

    for (uint32_t servo_index = 0; servo_index < SUPPORT_SERVO_COUNT; ++servo_index) {

        servo_config_t* servo_config = &servo_config_list[servo_index];

        // Calculate start address of servo configuration
        uint32_t base_address = MM_SERVO_CONFIG_BLOCK_BASE_EE_ADDRESS + servo_index * MM_SERVO_CONFIG_BLOCK_SIZE;

        // Read servo configuration
        config_read_8(base_address + MM_SERVO_CONFIG_OFFSET, &servo_config->config);
        if (servo_config->config == 0xFF) {
            return false;
        }
        
        // Load servo information
        uint8_t servo_id = (servo_config->config & MM_SERVO_CONFIG_SERVO_TYPE_MASK) >> 4;
        if (servo_id == SERVO_DS3218MG_270_ID) {
            servo_config->min_pulse_width  = DS3218MG_MIN_PULSE_WIDTH;
            servo_config->max_pulse_width  = DS3218MG_MAX_PULSE_WIDTH;
            servo_config->max_physic_angle = DS3218MG_MAX_PHYSIC_ANGLE;
        }
        else {
            // Unknown servo type
            return false;
        }

        // Read PWM channel
        config_read_8(base_address + MM_SERVO_PWM_CHANNEL_OFFSET, &servo_config->pwm_channel);
        if (servo_config->pwm_channel >= SUPPORT_PWM_CHANNELS_COUNT) {
            return false;
        }

        // Read protection min physic angle
        config_read_16(base_address + MM_SERVO_PROTECTION_MIN_PHYSIC_ANGLE_OFFSET, &servo_config->protection_min_physic_angle);
        if (servo_config->protection_min_physic_angle == 0xFFFF || servo_config->protection_min_physic_angle > servo_config->max_physic_angle) {
            return false;
        }

        // Read protection max physic angle
        config_read_16(base_address + MM_SERVO_PROTECTION_MAX_PHYSIC_ANGLE_OFFSET, &servo_config->protection_max_physic_angle);
        if (servo_config->protection_max_physic_angle == 0xFFFF || servo_config->protection_max_physic_angle > servo_config->max_physic_angle) {
            return false;
        }

        // Read servo logic zero
        config_read_16(base_address + MM_SERVO_LOGIC_ZERO_OFFSET, &servo_config->logic_zero);
        if (servo_config->logic_zero == 0xFFFF ||
            servo_config->logic_zero < servo_config->protection_min_physic_angle ||
            servo_config->logic_zero > servo_config->protection_max_physic_angle) {
            return false;
        }
    }
    return true;
}

//  ***************************************************************************
/// @brief  Start move servo to new angle
/// @param  ch:    servo channel
/// @param  angle: new angle
/// @return physic angle
//  ***************************************************************************
static float calculate_physic_angle(float logic_angle, const servo_config_t* config) {

    // Convert logic angle to physic angle
    float physic_angle = 0;
    if (config->config & MM_SERVO_CONFIG_REVERSE_DIRECTION_MASK) {
        physic_angle = config->logic_zero - logic_angle;      // Reverse rotate direction
    }
    else {
        physic_angle = config->logic_zero + logic_angle;      // Direct rotate direction
    }

    // Constrain physic servo angle (protection)
    if (physic_angle < config->protection_min_physic_angle) {
        physic_angle = config->protection_min_physic_angle;
    }
    if (physic_angle > config->protection_max_physic_angle) {
        physic_angle = config->protection_max_physic_angle;
    }

    return physic_angle;
}

//  ***************************************************************************
/// @brief  Convert servo angle to PWM pulse width
/// @param  physic_angle: servo angle
/// @param  config: servo configuration. @ref servo_config_t
/// @return PWM pulse width
//  ***************************************************************************
static uint32_t convert_angle_to_pulse_width(float physic_angle, const servo_config_t* servo_config) {

    float step = (float)(servo_config->max_pulse_width - servo_config->min_pulse_width) / (float)servo_config->max_physic_angle;
    float pulse_width = (float)servo_config->min_pulse_width + physic_angle * step;
    return (uint32_t)pulse_width;
}
