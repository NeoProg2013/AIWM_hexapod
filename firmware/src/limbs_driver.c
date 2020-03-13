//  ***************************************************************************
/// @file    limbs_driver.c
/// @author  NeoProg
//  ***************************************************************************
#include "limbs_driver.h"
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

#define MOTION_TIME_MAX_VALUE                      (100)
#define MOTION_TIME_START_VALUE                    (MOTION_TIME_MAX_VALUE / 2)


typedef enum {
    STATE_NOINIT,
    STATE_SYNC,
    STATE_CALCULATIONS,
    STATE_UPDATE_CONFIG
} state_t;

typedef enum {
    LINK_COXA,
    LINK_FEMUR,
    LINK_TIBIA
} link_id_t;

typedef enum {
    DIRECTION_REVERSE = -1,
    DIRECTION_HOLD = 0,
    DIRECTION_DIRECT = 1,
} direction_t;

typedef enum {
    OVERRIDE_LEVEL_NO,
    OVERRIDE_LEVEL_POSITION
} override_level_t;

/*typedef struct {
    float x;
    float y;
    float z;
} point_3d_t;*/

typedef struct {

    // Adaptive path algorithm
    point_3d_t start_position;      // Initialize once after system startup
    point_3d_t current_position;
    
    override_level_t override_level;
    point_3d_t override_position;

    // Inverse kinematic
    float    link_servo_angle[3];
    uint16_t link_length[3];        // Initialize once after system startup
    int16_t  link_zero_rotate[3];   // Initialize once after system startup

} limb_info_t;

typedef struct {
    direction_t direction;
    uint32_t speed;
    uint32_t step_length;
    int32_t  curvature;
} motion_config_t;


static bool read_configuration(void);
static bool calc_points_by_time(uint32_t time, const motion_config_t* motion_config, limb_info_t* limb_info);
static bool kinematic_calculate_angles(limb_info_t* limb_info);


static state_t core_state = STATE_NOINIT;
static limb_info_t limbs[SUPPORT_LIMBS_COUNT] = {0};
static motion_config_t current_motion_config;
static motion_config_t next_motion_config;



//  ***************************************************************************
/// @brief  Limbs driver initialize
/// @param  point_list: start points list
/// @return none
//  ***************************************************************************
void limbs_driver_init(const point_3d_t* point_list) {
    
    if (read_configuration() == false) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
        return;
    }
    
    // Initialize motion config to default values
    current_motion_config.direction   = next_motion_config.direction   = DIRECTION_HOLD;
    current_motion_config.speed       = next_motion_config.speed       = 1;
    current_motion_config.step_length = next_motion_config.step_length = 90;
    current_motion_config.curvature   = next_motion_config.curvature   = 1;
    
    // Set start points
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        limbs[i].start_position = limbs[i].current_position = point_list[i]; 
    }
    
    // Calculate start link angles
    if (kinematic_calculate_angles(limbs) == false) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
        return;
    }
    
    // Initialize servo driver
    servo_driver_init();
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        servo_driver_move(i * 3 + 0, limbs[i].link_servo_angle[LINK_COXA]);
        servo_driver_move(i * 3 + 1, limbs[i].link_servo_angle[LINK_FEMUR]);
        servo_driver_move(i * 3 + 2, limbs[i].link_servo_angle[LINK_TIBIA]);
    }
    
    // Initialization driver state
    //is_limbs_move_started = false;
    core_state = STATE_SYNC;
}

//  ***************************************************************************
/// @brief  Start smooth algorithm configuration
/// @param  point_count: smooth point count
//  ***************************************************************************
void limbs_driver_set_smooth_config(uint32_t point_count) {
    
    /*smooth_total_point_count = point_count;

    if (smooth_total_point_count == 0) {
        sysmon_set_error(SYSMON_FATAL_ERROR);
        sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
    }*/
}

//  ***************************************************************************
/// @brief  Start limb move
/// @param  point_list: destination point list
/// @param  path_type_list: path type list
//  ***************************************************************************
void limbs_driver_start_move(const point_3d_t* point_list, const path_type_t* path_type_list) {
    
    /*if (point_list == NULL) {
        sysmon_set_error(SYSMON_FATAL_ERROR);
        sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
        return;
    }
    
    // Prepare limbs for movement
    for (uint32_t i = 0; i < SUPPORT_LIMB_COUNT; ++i) {
        
        // Prepare limb for movement
        limbs[i].movement_path.path_type   = path_type_list[i];
        limbs[i].movement_path.start_point = limbs[i].position;
        limbs[i].movement_path.dest_point  = point_list[i];
        
        // Need start movement?
        if (limbs[i].position.x == point_list[i].x && limbs[i].position.y == point_list[i].y && limbs[i].position.z == point_list[i].z) {
            continue;
        }
        
        is_limbs_move_started = true;
    }*/
}

//  ***************************************************************************
/// @brief  Check all limbs movement complete
/// @param  none
/// @return true - move complete, false - movement in progress
//  ***************************************************************************
bool limbs_driver_is_move_complete(void) {
    
    //return is_limbs_move_started == false;
    return true;
}

//  ***************************************************************************
/// @brief  Limbs driver process
/// @note   Call from main loop
//  ***************************************************************************
void limbs_driver_process(void) {
    
    if (sysmon_is_module_disable(SYSMON_MODULE_LIMBS_DRIVER) == true) return;  // Module disabled
    

    static int32_t motion_time = MOTION_TIME_START_VALUE;
    static uint64_t prev_syncho_value = 0;

    static bool is_direct = true;
    switch (core_state) {

        case STATE_SYNC:
            if (synchro != prev_syncho_value) {

                if (synchro - prev_syncho_value != 0) {
                    // SYNC ERROR!!!
                }
                prev_syncho_value = synchro;
                core_state = STATE_CALCULATIONS;
            }
            break;

        case STATE_CALCULATIONS:
            calc_points_by_time(motion_time, &current_motion_config, limbs);
            kinematic_calculate_angles(limbs);

            for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
                servo_driver_move(i * 3 + 0, limbs[i].link_servo_angle[LINK_COXA]);
                servo_driver_move(i * 3 + 1, limbs[i].link_servo_angle[LINK_FEMUR]);
                servo_driver_move(i * 3 + 2, limbs[i].link_servo_angle[LINK_TIBIA]);
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
           /* if (time == 50) {
                core_state = STATE_UPDATE_CONFIG;
            }
            else {
                core_state = STATE_SYNC;
            }*/

            core_state = STATE_SYNC;
            break;

        case STATE_UPDATE_CONFIG:
            current_motion_config.direction   = next_motion_config.direction;
            current_motion_config.speed       = next_motion_config.speed;
            current_motion_config.step_length = next_motion_config.step_length;
            current_motion_config.curvature   = next_motion_config.curvature;
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
                limbs[limb_index].override_level,
                (int32_t)limbs[limb_index].override_position.x, 
                (int32_t)limbs[limb_index].override_position.y, 
                (int32_t)limbs[limb_index].override_position.z, 
                (int32_t)limbs[limb_index].current_position.x,  
                (int32_t)limbs[limb_index].current_position.y,  
                (int32_t)limbs[limb_index].current_position.z);
    }
    else if (strcmp(cmd, "set_override_level") == 0 && argc == 2) {

        // Set override level
        if (strcmp(argv[1], "no") == 0) {
           limbs[limb_index].override_level = OVERRIDE_LEVEL_NO;
           limbs[limb_index].override_position.x = 0;
           limbs[limb_index].override_position.y = 0;
           limbs[limb_index].override_position.z = 0;
        }
        else if (strcmp(argv[1], "position") == 0) {
           limbs[limb_index].override_level = OVERRIDE_LEVEL_POSITION;
           limbs[limb_index].override_position.x = limbs[limb_index].current_position.x;
           limbs[limb_index].override_position.y = limbs[limb_index].current_position.y;
           limbs[limb_index].override_position.z = limbs[limb_index].current_position.z;
        }
        else {
           return false;
        }
        sprintf(response, CLI_MSG("[%lu] has new override level %lu"), limb_index, limbs[limb_index].override_level);
    }
    else if (strcmp(cmd, "set_override_value") == 0 && argc == 4) {
        
        limbs[limb_index].override_position.x = atoi(argv[1]);
        limbs[limb_index].override_position.y = atoi(argv[2]);
        limbs[limb_index].override_position.z = atoi(argv[3]);
        sprintf(response, CLI_MSG("[%lu] has new override value %ld %ld %ld"), limb_index, 
                (int32_t)limbs[limb_index].override_position.x, 
                (int32_t)limbs[limb_index].override_position.y, 
                (int32_t)limbs[limb_index].override_position.z);
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
    if (config_read_16(base_address + MM_LIMB_COXA_LENGTH_OFFSET,  &length[LINK_COXA])  == false) return false;
    if (config_read_16(base_address + MM_LIMB_FEMUR_LENGTH_OFFSET, &length[LINK_FEMUR]) == false) return false;
    if (config_read_16(base_address + MM_LIMB_TIBIA_LENGTH_OFFSET, &length[LINK_TIBIA]) == false) return false;
    if (length[0] == 0xFFFF || length[1] == 0xFFFF || length[2] == 0xFFFF) {
        return false;
    }
    
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        limbs[i].link_length[LINK_COXA]  = length[LINK_COXA];
        limbs[i].link_length[LINK_FEMUR] = length[LINK_FEMUR];
        limbs[i].link_length[LINK_TIBIA] = length[LINK_TIBIA];
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
    limbs[0].link_zero_rotate[LINK_COXA] = 135;//coxa_zero_rotate_0_3;
    limbs[3].link_zero_rotate[LINK_COXA] = 180;//coxa_zero_rotate_0_3;
    limbs[1].link_zero_rotate[LINK_COXA] = 225;//coxa_zero_rotate_1_4;
    limbs[4].link_zero_rotate[LINK_COXA] = 315;//coxa_zero_rotate_1_4;
    limbs[2].link_zero_rotate[LINK_COXA] = 0;//coxa_zero_rotate_2_5;
    limbs[5].link_zero_rotate[LINK_COXA] = 45;//coxa_zero_rotate_2_5;
    
    // Read and set FEMUR, TIBIA 1-6 zero rotate
    int16_t femur_zero_rotate_femur = 0;
    int16_t tibia_zero_rotate_femur = 0;
    if (config_read_16(base_address + MM_LIMB_FEMUR_ZERO_ROTATE_OFFSET, (uint16_t*)&femur_zero_rotate_femur) == false ) return false;
    if (config_read_16(base_address + MM_LIMB_TIBIA_ZERO_ROTATE_OFFSET, (uint16_t*)&tibia_zero_rotate_femur) == false ) return false;
    if (abs(femur_zero_rotate_femur) > 360 || abs(tibia_zero_rotate_femur) > 360) {
        return false;
    }
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        limbs[i].link_zero_rotate[LINK_FEMUR] = femur_zero_rotate_femur;
        limbs[i].link_zero_rotate[LINK_TIBIA] = tibia_zero_rotate_femur;
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
/// @param  limb_info_list: @ref limb_info_t
/// @param  points_list: @ref point_3d_t
/// @retval limb_info_list::current_position
/// @return true - calculation success, false - no
//  ***************************************************************************
static bool calc_points_by_time(uint32_t time, const motion_config_t* motion_config, limb_info_t* limbs_info_list) {

    if (motion_config->curvature == 0 || motion_config->curvature > 1999 || motion_config->curvature < -1999) {
        return false; // Wrong curvature value
    }


    float curvature   = (float)motion_config->curvature / 1000.0f;
    float step_length = (float)motion_config->step_length;

    // Calculation radius of curvature
    float curvature_radius = tanf((2.0f - curvature) * M_PI / 4.0f) * step_length;



    //
    // Common calculations
    //
    float trajectory_radius[6] = {0};
    float start_angle_rad[6] = {0};
    float max_trajectory_radius = 0;
    for (uint32_t limb_id = 0; limb_id < SUPPORT_LIMBS_COUNT; ++limb_id) {

        // Save start point to separate variables for short code
        float x0 = limbs_info_list[limb_id].start_position.x;
        float z0 = limbs_info_list[limb_id].start_position.z;

        // Calculation trajectory radius
        trajectory_radius[limb_id] = sqrtf((curvature_radius - x0) * (curvature_radius - x0) + z0 * z0);

        // Search max trajectory radius
        if (trajectory_radius[limb_id] > max_trajectory_radius) {
            max_trajectory_radius = trajectory_radius[limb_id];
        }

        // Calculation limb start angle
        start_angle_rad[limb_id] = atan2f(z0, -(curvature_radius - x0));
    }
    if (max_trajectory_radius == 0) {
        return false; // Avoid division by zero
    }

    // Calculation max angle of arc
    int32_t curvature_radius_sign = (curvature_radius > 0) ? 1 : -1;
    float max_arc_angle = curvature_radius_sign * step_length / max_trajectory_radius;



    //
    // Calculation points by time
    //
    float move_time = ((float)time - 50.0f) / 100.0f;
    for (uint32_t limb_id = 0; limb_id < SUPPORT_LIMBS_COUNT; ++limb_id) {

        // Calculation arc angle for current time
        float arc_angle_rad = move_time * max_arc_angle + start_angle_rad[limb_id];

        // Calculation point by time
        limbs_info_list[limb_id].current_position.x = curvature_radius + trajectory_radius[limb_id] * cosf(arc_angle_rad);
        limbs_info_list[limb_id].current_position.z =                    trajectory_radius[limb_id] * sinf(arc_angle_rad);
    }

    return true;
}

//  ***************************************************************************
/// @brief  Calculate angles
/// @param  limbs_info_list: @ref limb_info_t
/// @retval limbs_info_list::servo_angle
/// @return true - calculation success, false - no
//  ***************************************************************************
static bool kinematic_calculate_angles(limb_info_t* limbs_info_list) {

    for (uint32_t limb_id = 0; limb_id < SUPPORT_LIMBS_COUNT; ++limb_id) {

        limb_info_t* info = &limbs_info_list[limb_id];

        float coxa_zero_rotate_deg  = info->link_zero_rotate[LINK_COXA];
        float femur_zero_rotate_deg = info->link_zero_rotate[LINK_FEMUR];
        float tibia_zero_rotate_deg = info->link_zero_rotate[LINK_TIBIA];
        float coxa_length  = info->link_length[LINK_COXA];
        float femur_length = info->link_length[LINK_FEMUR];
        float tibia_length = info->link_length[LINK_TIBIA];

        float x = info->current_position.x;
        float y = info->current_position.y;
        float z = info->current_position.z;


        // Move to (X*, Y*, Z*) coordinate system - rotate
        float coxa_zero_rotate_rad = DEG_TO_RAD(coxa_zero_rotate_deg);
        float x1 = x * cosf(coxa_zero_rotate_rad) + z * sinf(coxa_zero_rotate_rad);
        float y1 = y;
        float z1 = -x * sinf(coxa_zero_rotate_rad) + z * cosf(coxa_zero_rotate_rad);


        //
        // Calculate COXA angle
        //
        float coxa_angle_rad = atan2f(z1, x1);
        info->link_servo_angle[LINK_COXA] = RAD_TO_DEG(coxa_angle_rad);


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
        info->link_servo_angle[LINK_FEMUR] = femur_zero_rotate_deg - RAD_TO_DEG(alpha) - RAD_TO_DEG(fi);
        info->link_servo_angle[LINK_TIBIA] = RAD_TO_DEG(gamma) - tibia_zero_rotate_deg;
    }
    return true;
}
#undef M_PI
#undef RAD_TO_DEG
#undef DEG_TO_RAD
