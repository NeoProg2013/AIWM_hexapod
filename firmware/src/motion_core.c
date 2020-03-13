//  ***************************************************************************
/// @file    limbs_driver.c
/// @author  NeoProg
//  ***************************************************************************
#include "motion_core.h"
#include "servo_driver.h"
#include "configurator.h"
#include "systimer.h"
#include "pwm.h"
#include "system_monitor.h"
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SUPPORT_LIMBS_COUNT                 (6)
#define MOTION_TIME_SCALE                   (100)
#define MOTION_TIME_MAX_VALUE               (1 * MOTION_TIME_SCALE)
#define MOTION_TIME_START_VALUE             (MOTION_TIME_MAX_VALUE / 2)


typedef enum {
    STATE_NOINIT,
    STATE_SYNC,
    STATE_CALCULATIONS,
    STATE_UPDATE_CONFIG
} state_t;

typedef enum {
    DIRECTION_REVERSE = -1,
    DIRECTION_HOLD = 0,
    DIRECTION_DIRECT = 1,
} direction_t;

typedef enum {
    OVERRIDE_LEVEL_NO,
    OVERRIDE_LEVEL_POSITION
} override_level_t;

typedef enum {
    MOTION_STATE_START,
    MOTION_STATE_NORMAL,
    MOTION_STATE_STOP
} motion_stage_t;

typedef struct {
    float    angle;
    uint16_t length;      // Initialize once after system startup
    int16_t  zero_rotate; // Initialize once after system startup
} link_t;

typedef struct {
    bool is_stage_move_up;
    point_3d_t start_position; // Initialize once after system startup
    point_3d_t current_position;
    override_level_t override_level;
    point_3d_t override_position;
    link_t coxa;
    link_t femur;
    link_t tibia; 
} limb_t;

typedef struct {
    direction_t direction;
    uint32_t speed;
    uint32_t step_length;
    int32_t  curvature;
} motion_config_t;


static bool read_configuration(void);
static bool calc_points_by_time(uint32_t time, const motion_config_t* motion_config, limb_t* limbs_list);
static bool kinematic_calculate_angles(limb_t* limbs_list);


static state_t core_state = STATE_NOINIT;
static limb_t limbs_list[SUPPORT_LIMBS_COUNT] = {0};
static motion_config_t current_motion_config;
static motion_config_t next_motion_config;


//  ***************************************************************************
/// @brief  Motion core initialization
/// @param  none
/// @return none
//  ***************************************************************************
void motion_core_init(const point_3d_t* point_list) {
    
    if (read_configuration() == false) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
        return;
    }
    
    // Initialize motion config to default values
    current_motion_config.direction   = next_motion_config.direction   = DIRECTION_HOLD;
    current_motion_config.speed       = next_motion_config.speed       = 1;
    current_motion_config.step_length = next_motion_config.step_length = 90;
    current_motion_config.curvature   = next_motion_config.curvature   = 500;
    
    // Set start points
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        limbs_list[i].start_position = limbs_list[i].current_position = point_list[i];
        limbs_list[i].is_stage_move_up = ((i % 2) == 0);
    }
    
    // Calculate start link angles
    if (kinematic_calculate_angles(limbs_list) == false) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
        return;
    }
    
    // Initialize servo driver
    servo_driver_init();
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        servo_driver_move(i * 3 + 0, limbs_list[i].coxa.angle);
        servo_driver_move(i * 3 + 1, limbs_list[i].femur.angle);
        servo_driver_move(i * 3 + 2, limbs_list[i].tibia.angle);
    }
    
    // Initialize driver state
    core_state = STATE_SYNC;
}

//  ***************************************************************************
/// @brief  Limbs driver process
/// @note   Call from main loop
//  ***************************************************************************
void motion_core_process(void) {
    
    if (sysmon_is_module_disable(SYSMON_MODULE_LIMBS_DRIVER) == true) return;  // Module disabled
    

    static int32_t motion_time = MOTION_TIME_START_VALUE;
    static uint64_t prev_syncho_value = 0;

    static bool is_direct = true;
    switch (core_state) {

        case STATE_SYNC:
            if (synchro != prev_syncho_value) {

                if (synchro - prev_syncho_value != 0) {
                    sysmon_set_error(SYSMON_SYNC_ERROR);
                }
                prev_syncho_value = synchro;
                core_state = STATE_CALCULATIONS;
            }
            break;

        case STATE_CALCULATIONS:
            // Calculate points for adaptive trajectory
            if (calc_points_by_time(motion_time, &current_motion_config, limbs_list) == false) {
                sysmon_set_error(SYSMON_MATH_ERROR);
                sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
                return;
            }
            
            // Position override process
            for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
                if (limbs_list[i].override_level == OVERRIDE_LEVEL_POSITION) {
                    limbs_list[i].current_position = limbs_list[i].override_position;
                }
            }
            
            // Calculate links angles
            if (kinematic_calculate_angles(limbs_list) == false) {
                sysmon_set_error(SYSMON_MATH_ERROR);
                sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
                return;
            }

            // Load angles to servo driver
            for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
                servo_driver_move(i * 3 + 0, limbs_list[i].coxa.angle);
                servo_driver_move(i * 3 + 1, limbs_list[i].femur.angle);
                servo_driver_move(i * 3 + 2, limbs_list[i].tibia.angle);
            }


            
            if (motion_time >= MOTION_TIME_MAX_VALUE) {
                is_direct = false;
            }
            else if (motion_time <= 0) {
                is_direct = true;
            }
            if (is_direct) {
                motion_time += current_motion_config.speed;
            }
            else {
                motion_time -= current_motion_config.speed;
            }
            
            
            
            if (motion_time == MOTION_TIME_MAX_VALUE || motion_time == 0) {
                for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
                    limbs_list[i].is_stage_move_up = !limbs_list[i].is_stage_move_up;
                }
                //current_motion_stage = MOTION_STAGE_NORMAL;
            }
            
            core_state = STATE_SYNC;
            if (motion_time == MOTION_TIME_START_VALUE) {
                core_state = STATE_UPDATE_CONFIG;
            }
            break;

        case STATE_UPDATE_CONFIG:
            current_motion_config = next_motion_config;
            core_state = STATE_SYNC;
            break;

        case STATE_NOINIT:
        default:
            break;
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
bool motion_core_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response) {

    // Get limb index
    if (argc == 0) return false;
    uint32_t limb_index = atoi(argv[0]);
    if (limb_index > SUPPORT_LIMBS_COUNT) return false;


    //
    // Process command
    //
    if (strcmp(cmd, "status") == 0 && argc == 1) {

        sprintf(response, CLI_MSG("limb status report")
                          CLI_MSG("    - override level: %ld")
                          CLI_MSG("    - override position: %ld %ld %ld")
                          CLI_MSG("    - current position: %ld %ld %ld"),
                limbs_list[limb_index].override_level,
                (int32_t)limbs_list[limb_index].override_position.x, 
                (int32_t)limbs_list[limb_index].override_position.y, 
                (int32_t)limbs_list[limb_index].override_position.z, 
                (int32_t)limbs_list[limb_index].current_position.x,  
                (int32_t)limbs_list[limb_index].current_position.y,  
                (int32_t)limbs_list[limb_index].current_position.z);
    }
    else if (strcmp(cmd, "set_override_level") == 0 && argc == 2) {

        // Set override level
        if (strcmp(argv[1], "no") == 0) {
           limbs_list[limb_index].override_level = OVERRIDE_LEVEL_NO;
           limbs_list[limb_index].override_position.x = 0;
           limbs_list[limb_index].override_position.y = 0;
           limbs_list[limb_index].override_position.z = 0;
        }
        else if (strcmp(argv[1], "position") == 0) {
           limbs_list[limb_index].override_level = OVERRIDE_LEVEL_POSITION;
           limbs_list[limb_index].override_position.x = limbs_list[limb_index].current_position.x;
           limbs_list[limb_index].override_position.y = limbs_list[limb_index].current_position.y;
           limbs_list[limb_index].override_position.z = limbs_list[limb_index].current_position.z;
        }
        else {
           return false;
        }
        sprintf(response, CLI_MSG("[%lu] has new override level %lu"), limb_index, limbs_list[limb_index].override_level);
    }
    else if (strcmp(cmd, "set_override_value") == 0 && argc == 4) {
        
        limbs_list[limb_index].override_position.x = atoi(argv[1]);
        limbs_list[limb_index].override_position.y = atoi(argv[2]);
        limbs_list[limb_index].override_position.z = atoi(argv[3]);
        sprintf(response, CLI_MSG("[%lu] has new override value %ld %ld %ld"), limb_index, 
                (int32_t)limbs_list[limb_index].override_position.x, 
                (int32_t)limbs_list[limb_index].override_position.y, 
                (int32_t)limbs_list[limb_index].override_position.z);
    }
    else if (strcmp(cmd, "d") == 0 && argc == 1) {
        next_motion_config.direction = DIRECTION_DIRECT;
    }
    else if (strcmp(cmd, "h") == 0 && argc == 1) {
        next_motion_config.direction = DIRECTION_HOLD;
    }
    else if (strcmp(cmd, "r") == 0 && argc == 1) {
        next_motion_config.direction = DIRECTION_REVERSE;
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
    
    uint32_t base_address = MM_LIMB_CONFIG_BASE_EE_ADDRESS;
    
    // Read and set COXA, FEMUR, TIBIA length
    uint16_t length[3] = {0};
    if (config_read_16(base_address + MM_LIMB_COXA_LENGTH_OFFSET,  &length[0]) == false) return false;
    if (config_read_16(base_address + MM_LIMB_FEMUR_LENGTH_OFFSET, &length[1]) == false) return false;
    if (config_read_16(base_address + MM_LIMB_TIBIA_LENGTH_OFFSET, &length[2]) == false) return false;
    if (length[0] == 0xFFFF || length[1] == 0xFFFF || length[2] == 0xFFFF) {
        return false;
    }
    
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        limbs_list[i].coxa.length  = length[0];
        limbs_list[i].femur.length = length[1];
        limbs_list[i].tibia.length = length[2];
    }
    
    // Read and set COXA zero rotate
    int16_t coxa_zero_rotate_0_3 = 0;
    int16_t coxa_zero_rotate_1_4 = 0;
    int16_t coxa_zero_rotate_2_5 = 0;
    if (config_read_16(base_address + MM_LIMB_COXA_0_3_ZERO_ROTATE_OFFSET, (uint16_t*)&coxa_zero_rotate_0_3) == false) return false;
    if (config_read_16(base_address + MM_LIMB_COXA_1_4_ZERO_ROTATE_OFFSET, (uint16_t*)&coxa_zero_rotate_1_4) == false) return false;
    if (config_read_16(base_address + MM_LIMB_COXA_2_5_ZERO_ROTATE_OFFSET, (uint16_t*)&coxa_zero_rotate_2_5) == false) return false;
    if (abs(coxa_zero_rotate_0_3) > 360 || abs(coxa_zero_rotate_1_4) > 360 || abs(coxa_zero_rotate_2_5) > 360) {
        return false;
    }
    limbs_list[0].coxa.zero_rotate = 135;//coxa_zero_rotate_0_3;
    limbs_list[1].coxa.zero_rotate = 180;//coxa_zero_rotate_0_3;
    limbs_list[2].coxa.zero_rotate = 225;//coxa_zero_rotate_1_4;
    limbs_list[3].coxa.zero_rotate = 45;//coxa_zero_rotate_1_4;
    limbs_list[4].coxa.zero_rotate = 0;//coxa_zero_rotate_2_5;
    limbs_list[5].coxa.zero_rotate = 315;//coxa_zero_rotate_2_5;
    
    // Read and set FEMUR, TIBIA 1-6 zero rotate
    int16_t femur_zero_rotate_femur = 0;
    int16_t tibia_zero_rotate_femur = 0;
    if (config_read_16(base_address + MM_LIMB_FEMUR_ZERO_ROTATE_OFFSET, (uint16_t*)&femur_zero_rotate_femur) == false ) return false;
    if (config_read_16(base_address + MM_LIMB_TIBIA_ZERO_ROTATE_OFFSET, (uint16_t*)&tibia_zero_rotate_femur) == false ) return false;
    if (abs(femur_zero_rotate_femur) > 360 || abs(tibia_zero_rotate_femur) > 360) {
        return false;
    }
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        limbs_list[i].femur.zero_rotate = femur_zero_rotate_femur;
        limbs_list[i].tibia.zero_rotate = tibia_zero_rotate_femur;
    }
    
    return true;
}

#define M_PI                                (3.14159265f)
#define RAD_TO_DEG(rad)                     ((rad) * 180.0f / M_PI)
#define DEG_TO_RAD(deg)                     ((deg) * M_PI / 180.0f)
//  ***************************************************************************
/// @brief  Calculate limbs coordinates according trajectory parameters
/// @param  time: current movement time [0; 100]
/// @param  motion_config: @ref motion_config_t
/// @param  limbs_list: @ref limb_t
/// @retval limbs_list::current_position
/// @return true - calculation success, false - no
//  ***************************************************************************
static bool calc_points_by_time(uint32_t time, const motion_config_t* motion_config, limb_t* limbs_list) {
    
    // Hexapod hold current position - just copy start coordinates to current
    if (motion_config->direction == DIRECTION_HOLD) {
        for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
            limbs_list[i].current_position = limbs_list[i].start_position;
        }
        return true;
    }
    

    if (motion_config->curvature == 0 || motion_config->curvature > 1999 || motion_config->curvature < -1999) {
        return false; // Wrong curvature value
    }
    
    //
    // Calculate XZ
    //
    float curvature   = (float)motion_config->curvature / 1000.0f;
    float step_length = (float)motion_config->step_length;

    // Calculation radius of curvature
    float curvature_radius = tanf((2.0f - curvature) * M_PI / 4.0f) * step_length;

    // Common calculations
    float trajectory_radius[SUPPORT_LIMBS_COUNT] = {0};
    float start_angle_rad[SUPPORT_LIMBS_COUNT] = {0};
    float max_trajectory_radius = 0;
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {

        // Save start point to separate variables for short code
        float x0 = limbs_list[i].start_position.x;
        float z0 = limbs_list[i].start_position.z;

        // Calculation trajectory radius
        trajectory_radius[i] = sqrtf((curvature_radius - x0) * (curvature_radius - x0) + z0 * z0);

        // Search max trajectory radius
        if (trajectory_radius[i] > max_trajectory_radius) {
            max_trajectory_radius = trajectory_radius[i];
        }

        // Calculation limb start angle
        start_angle_rad[i] = atan2f(z0, -(curvature_radius - x0));
    }
    if (max_trajectory_radius == 0) {
        return false; // Avoid division by zero
    }

    // Calculation max angle of arc
    int32_t curvature_radius_sign = (curvature_radius > 0) ? 1 : -1;
    float max_arc_angle = curvature_radius_sign * step_length / max_trajectory_radius;

    // Calculation points by time
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        
        float motion_time = 0;
        if ((i % 2) == 0) {
            motion_time = -((float)time / MOTION_TIME_SCALE - 0.5f) * (float)motion_config->direction;
        }
        else {
            motion_time = +((float)time / MOTION_TIME_SCALE - 0.5f) * (float)motion_config->direction;
        }

        // Calculation arc angle for current time
        float arc_angle_rad = motion_time * max_arc_angle + start_angle_rad[i];

        // Calculation point by time
        limbs_list[i].current_position.x = curvature_radius + trajectory_radius[i] * cosf(arc_angle_rad);
        limbs_list[i].current_position.z =                    trajectory_radius[i] * sinf(arc_angle_rad);
    }
    
    //
    // Calculate Y
    //
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        
        if (limbs_list[i].is_stage_move_up) {
            float angle = M_PI * (float)time / MOTION_TIME_SCALE;
            limbs_list[i].current_position.y = limbs_list[i].start_position.y + sin(angle) * 25;
        }
        else {
            limbs_list[i].current_position.y = limbs_list[i].start_position.y;
        }
    }
    
    return true;
}

//  ***************************************************************************
/// @brief  Calculate angles
/// @param  limbs_list: @ref limb_t
/// @retval limbs_list::servo_angle
/// @return true - calculation success, false - no
//  ***************************************************************************
static bool kinematic_calculate_angles(limb_t* limbs_list) {

    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {

        float coxa_zero_rotate_deg  = limbs_list[i].coxa.zero_rotate;
        float femur_zero_rotate_deg = limbs_list[i].femur.zero_rotate;
        float tibia_zero_rotate_deg = limbs_list[i].tibia.zero_rotate;
        float coxa_length  = limbs_list[i].coxa.length;
        float femur_length = limbs_list[i].femur.length;
        float tibia_length = limbs_list[i].tibia.length;

        float x = limbs_list[i].current_position.x;
        float y = limbs_list[i].current_position.y;
        float z = limbs_list[i].current_position.z;


        // Move to (X*, Y*, Z*) coordinate system - rotate
        float coxa_zero_rotate_rad = DEG_TO_RAD(coxa_zero_rotate_deg);
        float x1 = x * cosf(coxa_zero_rotate_rad) + z * sinf(coxa_zero_rotate_rad);
        float y1 = y;
        float z1 = -x * sinf(coxa_zero_rotate_rad) + z * cosf(coxa_zero_rotate_rad);


        //
        // Calculate COXA angle
        //
        float coxa_angle_rad = atan2f(z1, x1);
        limbs_list[i].coxa.angle = RAD_TO_DEG(coxa_angle_rad);


        //
        // Prepare for calculation FEMUR and TIBIA angles
        //
        // Move to (X*, Y*) coordinate system (rotate on axis Y)
        x1 = x1 * cosf(coxa_angle_rad) + z1 * sinf(coxa_angle_rad);

        // Move to (X**, Y**) coordinate system (remove coxa from calculations)
        x1 = x1 - coxa_length;

        // Calculate angle between axis X and destination point
        float fi = atan2f(y1, x1);

        // Calculate distance to destination point
        float d = sqrt(x1 * x1 + y1 * y1);
        if (d > femur_length + tibia_length) {
            return false; // Point not attainable
        }


        //
        // Calculate triangle angles
        //
        float a = tibia_length;
        float b = femur_length;
        float c = d;

        float alpha = acosf( (b * b + c * c - a * a) / (2.0f * b * c) );
        float gamma = acosf( (a * a + b * b - c * c) / (2.0f * a * b) );


        //
        // Calculate FEMUR and TIBIA angle
        //
        limbs_list[i].femur.angle = femur_zero_rotate_deg - RAD_TO_DEG(alpha) - RAD_TO_DEG(fi);
        limbs_list[i].tibia.angle = RAD_TO_DEG(gamma) - tibia_zero_rotate_deg;
    }
    return true;
}
#undef M_PI
#undef RAD_TO_DEG
#undef DEG_TO_RAD
