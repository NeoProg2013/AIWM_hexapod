//  ***************************************************************************
/// @file    motion_core.c
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


typedef enum {
    STATE_NOINIT,
    STATE_SYNC,
    STATE_CALC,
    STATE_TIME_SHIFT,
} core_state_t;

typedef struct {
    float    angle;
    uint16_t length;
    int16_t  zero_rotate;
    
    int16_t  prot_min_angle; // Protection min angle, [degree]
    int16_t  prot_max_angle; // Protection max angle, [degree]
} link_t;

typedef struct {
    point_3d_t position;
    link_t coxa;
    link_t femur;
    link_t tibia;
} limb_t;

typedef struct {
    int32_t curvature;
    int32_t distance;
} traejctory_config_t;


static bool read_configuration(void);
static bool process_linear_trajectory(float motion_time);
static bool process_advanced_trajectory(float motion_time);
static bool kinematic_calculate_angles(void);


static core_state_t g_core_state = STATE_NOINIT;
static limb_t g_limbs_list[SUPPORT_LIMBS_COUNT] = {0};

static motion_config_t g_motion_config = {0};
static traejctory_config_t g_current_trajectory_config = {0};
static traejctory_config_t g_next_trajectory_config = {0};
static bool is_trajectory_config_init = false;



//  ***************************************************************************
/// @brief  Motion core initialization
/// @param  start_point_list: limbs start points list
/// @return none
//  ***************************************************************************
void motion_core_init(const point_3d_t* start_point_list) {

    if (read_configuration() == false) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
        return;
    }
    
    // Set start points
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        g_limbs_list[i].position = start_point_list[i];
    }
    
    // Calculate start link angles
    if (kinematic_calculate_angles() == false) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
    }
    
    // Initialize servo driver
    servo_driver_init();
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        servo_driver_move(i * 3 + 0, g_limbs_list[i].coxa.angle);
        servo_driver_move(i * 3 + 1, g_limbs_list[i].femur.angle);
        servo_driver_move(i * 3 + 2, g_limbs_list[i].tibia.angle);
    }
    
    // Initialize driver state
    g_core_state = STATE_SYNC;
}

//  ***************************************************************************
/// @brief  Start motion
/// @param  motion_config: motion configuration. @ref motion_config_t
/// @return none
//  ***************************************************************************
void motion_core_start_motion(const motion_config_t* motion_config) {
    
    // Initialize motion configuration
    g_motion_config = *motion_config;
    if (g_motion_config.is_need_init_start_position) {
        for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
            g_motion_config.start_positions[i] = g_limbs_list[i].position;
        }
    }
    
    // Initialize trajectory configuration
    if (is_trajectory_config_init == false) {
        g_current_trajectory_config = g_next_trajectory_config;
        is_trajectory_config_init = true;
    }
}

//  ***************************************************************************
/// @brief  Reset trajectory configuration
/// @param  none
/// @return none
//  ***************************************************************************
void motion_core_reset_trajectory_config(void) {
    g_current_trajectory_config.curvature = 1;
    g_current_trajectory_config.distance = 0;
    is_trajectory_config_init = false;
}

//  ***************************************************************************
/// @brief  Update trajectory configuration
/// @param  curvature: curvature (parameter of trajectory)
/// @param  distance: distance (parameter of trajectory)
/// @return none
//  ***************************************************************************
void motion_core_update_trajectory_config(int32_t curvature, int32_t distance) {
    g_next_trajectory_config.curvature = curvature;
    g_next_trajectory_config.distance = distance;
}

//  ***************************************************************************
/// @brief  Motion core process
/// @param  none
/// @return none
//  ***************************************************************************
void motion_core_process(void) {

    if (sysmon_is_module_disable(SYSMON_MODULE_MOTION_DRIVER) == true) return;  // Module disabled

    
    static uint64_t prev_synchro_value = 0;
    float scaled_motion_time = 0;
    switch (g_core_state) {

        case STATE_SYNC:
            if (synchro != prev_synchro_value) {
                if (synchro - prev_synchro_value > 1 && prev_synchro_value != 0) {
                    sysmon_set_error(SYSMON_SYNC_ERROR);
                }
                prev_synchro_value = synchro;
                g_core_state = STATE_CALC;
            }
            break;

        case STATE_CALC:
            if (g_motion_config.motion_time >= g_motion_config.time_stop) {
                g_core_state = STATE_SYNC;
                break;
            }
            
            // Calculate new limbs positions
            scaled_motion_time = (float)g_motion_config.motion_time / (float)MTIME_SCALE; // to [0; 1]
            if (process_linear_trajectory(scaled_motion_time) == false) {
                sysmon_set_error(SYSMON_MATH_ERROR);
                sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
                break;
            }
            if (process_advanced_trajectory(scaled_motion_time) == false) {
                sysmon_set_error(SYSMON_MATH_ERROR);
                sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
                break;
            }
            
            // Calculate servo angles
            if (kinematic_calculate_angles() == false) {
                sysmon_set_error(SYSMON_MATH_ERROR);
                sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
                break;
            }
            
            // Load new angles to servo driver
            for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
                servo_driver_move(i * 3 + 0, g_limbs_list[i].coxa.angle);
                servo_driver_move(i * 3 + 1, g_limbs_list[i].femur.angle);
                servo_driver_move(i * 3 + 2, g_limbs_list[i].tibia.angle);
            }

            g_core_state = STATE_TIME_SHIFT;
            break;

        case STATE_TIME_SHIFT:
            g_motion_config.motion_time += g_motion_config.time_step;
            if (g_motion_config.motion_time == g_motion_config.time_update) {
                g_current_trajectory_config = g_next_trajectory_config;
            }
            g_core_state = STATE_SYNC;
            break;

        case STATE_NOINIT:
        default:
            sysmon_set_error(SYSMON_FATAL_ERROR);
            sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
            break;
    }
}

//  ***************************************************************************
/// @brief  Check motion status
/// @param  none
/// @return true - motion completed, false - motion in progress
//  ***************************************************************************
bool motion_core_is_motion_complete(void) {
    return g_motion_config.motion_time >= g_motion_config.time_stop;
}





//  ***************************************************************************
/// @brief  Read configuration
/// @param  none
/// @return true - read success, false - fail
//  ***************************************************************************
static bool read_configuration(void) {
    
    uint32_t base_address = MM_LIMB_CONFIG_BASE_EE_ADDRESS;
    
    // Read length
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (config_read_16(base_address + MM_LIMB_COXA_LENGTH_OFFSET,  &g_limbs_list[i].coxa.length) == false) return false;
        if (config_read_16(base_address + MM_LIMB_FEMUR_LENGTH_OFFSET, &g_limbs_list[i].femur.length) == false) return false;
        if (config_read_16(base_address + MM_LIMB_TIBIA_LENGTH_OFFSET, &g_limbs_list[i].tibia.length) == false) return false;
    }
    
    // Read zero rotate
    //int16_t femur_zero_rotate_femur = 35;
    //int16_t tibia_zero_rotate_femur = 135;
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (config_read_16(base_address + MM_LIMB_COXA_ZERO_ROTATE_OFFSET + i * sizeof(uint16_t), (uint16_t*)&g_limbs_list[i].coxa.zero_rotate) == false) return false;
        if (config_read_16(base_address + MM_LIMB_FEMUR_ZERO_ROTATE_OFFSET, (uint16_t*)&g_limbs_list[i].femur.zero_rotate) == false ) return false;
        if (config_read_16(base_address + MM_LIMB_TIBIA_ZERO_ROTATE_OFFSET, (uint16_t*)&g_limbs_list[i].tibia.zero_rotate) == false ) return false;
        if (abs(g_limbs_list[i].coxa.zero_rotate) > 360 || abs(g_limbs_list[i].femur.zero_rotate) > 360 || abs(g_limbs_list[i].tibia.zero_rotate) > 360) {
            return false;
        }
    }
    
    // Read protection
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (config_read_16(base_address + MM_LIMB_PROTECTION_COXA_MIN_ANGLE_OFFSET,  (uint16_t*)&g_limbs_list[i].coxa.prot_min_angle)  == false ) return false;
        if (config_read_16(base_address + MM_LIMB_PROTECTION_COXA_MAX_ANGLE_OFFSET,  (uint16_t*)&g_limbs_list[i].coxa.prot_max_angle)  == false ) return false;
        if (config_read_16(base_address + MM_LIMB_PROTECTION_FEMUR_MIN_ANGLE_OFFSET, (uint16_t*)&g_limbs_list[i].femur.prot_min_angle) == false ) return false;
        if (config_read_16(base_address + MM_LIMB_PROTECTION_FEMUR_MAX_ANGLE_OFFSET, (uint16_t*)&g_limbs_list[i].femur.prot_max_angle) == false ) return false;
        if (config_read_16(base_address + MM_LIMB_PROTECTION_TIBIA_MIN_ANGLE_OFFSET, (uint16_t*)&g_limbs_list[i].tibia.prot_min_angle) == false ) return false;
        if (config_read_16(base_address + MM_LIMB_PROTECTION_TIBIA_MAX_ANGLE_OFFSET, (uint16_t*)&g_limbs_list[i].tibia.prot_max_angle) == false ) return false;
        if (g_limbs_list[i].coxa.prot_min_angle  >= g_limbs_list[i].coxa.prot_max_angle  || 
            g_limbs_list[i].femur.prot_min_angle >= g_limbs_list[i].femur.prot_max_angle || 
            g_limbs_list[i].tibia.prot_min_angle >= g_limbs_list[i].tibia.prot_max_angle) {
            return false;
        }
    }

    return true;
}

#define M_PI                                (3.14159265f)
#define RAD_TO_DEG(rad)                     ((rad) * 180.0f / M_PI)
#define DEG_TO_RAD(deg)                     ((deg) * M_PI / 180.0f)


//  ***************************************************************************
/// @brief  Process linear trajectory
/// @param  motion_time: current motion time [0; 1]
/// @retval modify g_limbs_list
/// @return true - calculation success, false - no
//  ***************************************************************************
static bool process_linear_trajectory(float motion_time) {
    
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        
        // Skip limbs which not use linear trajectory
        if (g_motion_config.trajectories[i] != TRAJECTORY_XYZ_LINEAR) {
            continue;
        }
      
        // Inversion motion time if need
        float relative_motion_time = motion_time;
        if (g_motion_config.time_directions[i] == TIME_DIR_REVERSE) {
            relative_motion_time = 1.0f - relative_motion_time;
        }
        
        float x0 = g_motion_config.start_positions[i].x;
        float y0 = g_motion_config.start_positions[i].y;
        float z0 = g_motion_config.start_positions[i].z;
        float x1 = g_motion_config.dest_positions[i].x;
        float y1 = g_motion_config.dest_positions[i].y;
        float z1 = g_motion_config.dest_positions[i].z;
        
        g_limbs_list[i].position.x = x0 + relative_motion_time * (x1 - x0);
        g_limbs_list[i].position.y = y0 + relative_motion_time * (y1 - y0);
        g_limbs_list[i].position.z = z0 + relative_motion_time * (z1 - z0);
    }
    
    return true;
}

//  ***************************************************************************
/// @brief  Process advanced trajectory
/// @param  motion_time: current motion time [0; 1]
/// @retval modify g_limbs_list
/// @return true - calculation success, false - no
//  ***************************************************************************
static bool process_advanced_trajectory(float motion_time) {
    
    // Check need process advanced trajectory
    bool is_need_process = false;
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (g_motion_config.trajectories[i] == TRAJECTORY_XZ_ADV_Y_CONST || 
            g_motion_config.trajectories[i] == TRAJECTORY_XZ_ADV_Y_SINUS) {
            is_need_process = true;
            break;
        }
    }
    if (is_need_process == false) {
        return true;
    }
    
    // Check curvature value
    float curvature = (float)g_current_trajectory_config.curvature / 1000.0f;
    if (g_current_trajectory_config.curvature == 0)    curvature = +0.001f;
    if (g_current_trajectory_config.curvature > 1999)  curvature = +1.999f;
    if (g_current_trajectory_config.curvature < -1999) curvature = -1.999f;
    
    //
    // Calculate XZ
    //
    float distance = (float)g_current_trajectory_config.distance;

    // Calculation radius of curvature
    float curvature_radius = tanf((2.0f - curvature) * M_PI / 4.0f) * distance;

    // Common calculations
    float trajectory_radius[SUPPORT_LIMBS_COUNT] = {0};
    float start_angle_rad[SUPPORT_LIMBS_COUNT] = {0};
    float max_trajectory_radius = 0;
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        
        // Skip limbs which not use advanced trajectory
        if (g_motion_config.trajectories[i] != TRAJECTORY_XZ_ADV_Y_CONST && 
            g_motion_config.trajectories[i] != TRAJECTORY_XZ_ADV_Y_SINUS) {
            continue;
        }

        // Save start point to separate variables for short code
        float x0 = g_motion_config.start_positions[i].x;
        float z0 = g_motion_config.start_positions[i].z;

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
    int32_t curvature_radius_sign = (curvature_radius >= 0) ? 1 : -1;
    float max_arc_angle = curvature_radius_sign * distance / max_trajectory_radius;

    // Calculation points by time
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        
        // Skip limbs which not use advanced trajectory
        if (g_motion_config.trajectories[i] != TRAJECTORY_XZ_ADV_Y_CONST && 
            g_motion_config.trajectories[i] != TRAJECTORY_XZ_ADV_Y_SINUS) {
            continue;
        }
        
        // Inversion motion time if need
        float relative_motion_time = motion_time;
        if (g_motion_config.time_directions[i] == TIME_DIR_REVERSE) {
            relative_motion_time = 1.0f - relative_motion_time;
        }

        // Calculation arc angle for current time
        float arc_angle_rad = (relative_motion_time - 0.5f) * max_arc_angle + start_angle_rad[i];

        // Calculation XZ points by time
        g_limbs_list[i].position.x = curvature_radius + trajectory_radius[i] * cosf(arc_angle_rad);
        g_limbs_list[i].position.z =                    trajectory_radius[i] * sinf(arc_angle_rad);
        
        
        
        // Calculation Y points by time
        if (g_motion_config.trajectories[i] == TRAJECTORY_XZ_ADV_Y_CONST) {
            g_limbs_list[i].position.y = g_motion_config.start_positions[i].y;
        }
        else if (g_motion_config.trajectories[i] == TRAJECTORY_XZ_ADV_Y_SINUS) {
            g_limbs_list[i].position.y = g_motion_config.start_positions[i].y;
            g_limbs_list[i].position.y += LIMB_STEP_HEIGHT * sinf(relative_motion_time * M_PI);  
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
static bool kinematic_calculate_angles(void) {

    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {

        float coxa_zero_rotate_deg  = g_limbs_list[i].coxa.zero_rotate;
        float femur_zero_rotate_deg = g_limbs_list[i].femur.zero_rotate;
        float tibia_zero_rotate_deg = g_limbs_list[i].tibia.zero_rotate;
        float coxa_length  = g_limbs_list[i].coxa.length;
        float femur_length = g_limbs_list[i].femur.length;
        float tibia_length = g_limbs_list[i].tibia.length;

        float x = g_limbs_list[i].position.x;
        float y = g_limbs_list[i].position.y;
        float z = g_limbs_list[i].position.z;


        // Move to (X*, Y*, Z*) coordinate system - rotate
        float coxa_zero_rotate_rad = DEG_TO_RAD(coxa_zero_rotate_deg);
        float x1 = x * cosf(coxa_zero_rotate_rad) + z * sinf(coxa_zero_rotate_rad);
        float y1 = y;
        float z1 = -x * sinf(coxa_zero_rotate_rad) + z * cosf(coxa_zero_rotate_rad);


        //
        // Calculate COXA angle
        //
        float coxa_angle_rad = atan2f(z1, x1);
        g_limbs_list[i].coxa.angle = RAD_TO_DEG(coxa_angle_rad);


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
        g_limbs_list[i].femur.angle = femur_zero_rotate_deg - RAD_TO_DEG(alpha) - RAD_TO_DEG(fi);
        g_limbs_list[i].tibia.angle = RAD_TO_DEG(gamma) - tibia_zero_rotate_deg;
        
        //
        // Protection
        //
        if (g_limbs_list[i].coxa.angle < g_limbs_list[i].coxa.prot_min_angle)   g_limbs_list[i].coxa.angle = g_limbs_list[i].coxa.prot_min_angle;
        if (g_limbs_list[i].coxa.angle > g_limbs_list[i].coxa.prot_max_angle)   g_limbs_list[i].coxa.angle = g_limbs_list[i].coxa.prot_max_angle;
        if (g_limbs_list[i].femur.angle < g_limbs_list[i].femur.prot_min_angle) g_limbs_list[i].femur.angle = g_limbs_list[i].femur.prot_min_angle;
        if (g_limbs_list[i].femur.angle > g_limbs_list[i].femur.prot_max_angle) g_limbs_list[i].femur.angle = g_limbs_list[i].femur.prot_max_angle;
        if (g_limbs_list[i].tibia.angle < g_limbs_list[i].tibia.prot_min_angle) g_limbs_list[i].tibia.angle = g_limbs_list[i].tibia.prot_min_angle;
        if (g_limbs_list[i].tibia.angle > g_limbs_list[i].tibia.prot_max_angle) g_limbs_list[i].tibia.angle = g_limbs_list[i].tibia.prot_max_angle;
    }
    return true;
}
#undef M_PI
#undef RAD_TO_DEG
#undef DEG_TO_RAD
