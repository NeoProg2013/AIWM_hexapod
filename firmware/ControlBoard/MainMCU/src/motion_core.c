//  ***************************************************************************
/// @file    motion_core.c
/// @author  NeoProg
//  ***************************************************************************
#include "motion_core.h"
#include "project_base.h"
#include "servo_driver.h"
#include "configurator.h"
#include "systimer.h"
#include "pwm.h"
#include "smcu.h"
#include "system_monitor.h"
#include <math.h>


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


static bool read_configuration(void);
static bool process_linear_trajectory(float motion_time);
static bool process_advanced_trajectory(float motion_time);
static bool kinematic_calculate_angles(void);


static limb_t g_limbs[SUPPORT_LIMBS_COUNT] = {0};

static motion_t g_current_motion = {0};
static motion_config_t g_current_motion_config = {0};
static motion_config_t g_next_motion_config = {0};

static bool is_enable_data_logging = false;



//  ***************************************************************************
/// @brief  Motion core initialization
/// @param  start_points: limbs start points list
//  ***************************************************************************
void motion_core_init(const point_3d_t* start_points) {
    if (read_configuration() == false) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
        return;
    }
    
    // Set start points
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        g_limbs[i].position = start_points[i];
    }
    
    // Calculate start link angles
    if (kinematic_calculate_angles() == false) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
        // Do not return - need init servo driver for CLI access
    }
    
    // Initialize servo driver
    servo_driver_init();
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        servo_driver_move(i * 3 + 0, g_limbs[i].coxa.angle);
        servo_driver_move(i * 3 + 1, g_limbs[i].femur.angle);
        servo_driver_move(i * 3 + 2, g_limbs[i].tibia.angle);
    }
    
    // Enable servo power
    if (sysmon_is_module_disable(SYSMON_MODULE_MOTION_DRIVER) == false) {
        servo_driver_power_on();
    }
}

//  ***************************************************************************
/// @brief  Start motion
/// @param  motion: motion description. @ref motion_t
//  ***************************************************************************
void motion_core_start_motion(const motion_t* motion) {
    g_current_motion = *motion;
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (g_current_motion.trajectories[i] == TRAJECTORY_XYZ_LINEAR) { // Set start point for linear traertory
            g_current_motion.start_positions[i] = g_limbs[i].position;
        }
    }
}

//  ***************************************************************************
/// @brief  Reset motion configuration
/// @note   Call before select new sequence
/// @param  motion_config: motion configuration
//  ***************************************************************************
void motion_core_init_motion_config(const motion_config_t* motion_config) {
    g_next_motion_config = *motion_config;
    g_current_motion_config = *motion_config;
    servo_driver_set_speed(motion_config->speed);
}

//  ***************************************************************************
/// @brief  Set new motion configuration
/// @param  motion_config: motion configuration
//  ***************************************************************************
void motion_core_update_motion_config(const motion_config_t* motion_config) {
    g_next_motion_config = *motion_config;
}

//  ***************************************************************************
/// @brief  Motion core process
/// @note   Call each PWM period
//  ***************************************************************************
void motion_core_process(void) {
    if (sysmon_is_module_disable(SYSMON_MODULE_MOTION_DRIVER) == true) return;  // Module disabled


    if (g_current_motion.motion_time >= g_current_motion.time_stop) {
        return; // We reach to end of motion - nothing do
    }
    
    // Scale time to [0.0; 1.0] range
    float scaled_motion_time = (float)g_current_motion.motion_time / (float)MTIME_SCALE;
    
    // Gatherig sensor data
    int16_t* foot_sensors_data = NULL;
    int16_t* accel_sensor_data = NULL;
    smcu_get_sensor_data(&foot_sensors_data, &accel_sensor_data);
    
    // Calculate new limbs positions
    if (process_linear_trajectory(scaled_motion_time) == false) {
        sysmon_set_error(SYSMON_MATH_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
        return;
    }
    if (process_advanced_trajectory(scaled_motion_time) == false) {
        sysmon_set_error(SYSMON_MATH_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
        return;
    }
    
    // Calculate servo logic angles
    if (kinematic_calculate_angles() == false) {
        sysmon_set_error(SYSMON_MATH_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
        return;
    }
    
    // Load new angles to servo driver
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        servo_driver_move(i * 3 + 0, g_limbs[i].coxa.angle);
        servo_driver_move(i * 3 + 1, g_limbs[i].femur.angle);
        servo_driver_move(i * 3 + 2, g_limbs[i].tibia.angle);
    }

    // Time shift and load new trajectory configuration if need
    g_current_motion.motion_time += g_current_motion.time_step;
    if (g_current_motion.motion_time == g_current_motion.time_update) {
        g_current_motion_config = g_next_motion_config;
        servo_driver_set_speed(g_current_motion_config.speed);
    }
    
    
    if (is_enable_data_logging) {
        void* tx_buffer = cli_get_tx_buffer();
        sprintf(tx_buffer, "[MOTION CORE]: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", 
                (int16_t)g_limbs[0].position.x * 10, (int16_t)g_limbs[0].position.y * 10, (int16_t)g_limbs[0].position.z * 10, foot_sensors_data[0], 
                (int16_t)g_limbs[1].position.x * 10, (int16_t)g_limbs[1].position.y * 10, (int16_t)g_limbs[1].position.z * 10, foot_sensors_data[1], 
                (int16_t)g_limbs[2].position.x * 10, (int16_t)g_limbs[2].position.y * 10, (int16_t)g_limbs[2].position.z * 10, foot_sensors_data[2], 
                (int16_t)g_limbs[3].position.x * 10, (int16_t)g_limbs[3].position.y * 10, (int16_t)g_limbs[3].position.z * 10, foot_sensors_data[3], 
                (int16_t)g_limbs[4].position.x * 10, (int16_t)g_limbs[4].position.y * 10, (int16_t)g_limbs[4].position.z * 10, foot_sensors_data[4], 
                (int16_t)g_limbs[5].position.x * 10, (int16_t)g_limbs[5].position.y * 10, (int16_t)g_limbs[5].position.z * 10, foot_sensors_data[5], 
                accel_sensor_data[0], accel_sensor_data[1], accel_sensor_data[2]);
        cli_send_data(NULL);
    }
}

//  ***************************************************************************
/// @brief  Check motion status
/// @return true - motion completed, false - motion in progress
//  ***************************************************************************
bool motion_core_is_motion_complete(void) {
    return g_current_motion.motion_time >= g_current_motion.time_stop;
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
    if (strcmp(cmd, "logging") == 0 && argc == 1) {
        is_enable_data_logging = (argv[0][0] == '1');
    } else {
        strcpy(response, CLI_ERROR("Unknown command or format for servo driver"));
        return false;
    }
    return true;
}





//  ***************************************************************************
/// @brief  Read configuration
/// @return true - read success, false - fail
//  ***************************************************************************
static bool read_configuration(void) {
    uint32_t base_address = MM_LIMB_CONFIG_BASE_EE_ADDRESS;
    
    // Read length
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (config_read_16(base_address + MM_LIMB_COXA_LENGTH_OFFSET,  &g_limbs[i].coxa.length)  == false) return false;
        if (config_read_16(base_address + MM_LIMB_FEMUR_LENGTH_OFFSET, &g_limbs[i].femur.length) == false) return false;
        if (config_read_16(base_address + MM_LIMB_TIBIA_LENGTH_OFFSET, &g_limbs[i].tibia.length) == false) return false;
    }
    
    // Read zero rotate
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (config_read_16(base_address + MM_LIMB_COXA_ZERO_ROTATE_OFFSET + i * sizeof(uint16_t), (uint16_t*)&g_limbs[i].coxa.zero_rotate) == false) return false;
        if (config_read_16(base_address + MM_LIMB_FEMUR_ZERO_ROTATE_OFFSET, (uint16_t*)&g_limbs[i].femur.zero_rotate) == false) return false;
        if (config_read_16(base_address + MM_LIMB_TIBIA_ZERO_ROTATE_OFFSET, (uint16_t*)&g_limbs[i].tibia.zero_rotate) == false) return false;
        if (abs(g_limbs[i].coxa.zero_rotate) > 360 || abs(g_limbs[i].femur.zero_rotate) > 360 || abs(g_limbs[i].tibia.zero_rotate) > 360) {
            return false;
        }
    }
    
    // Read protection
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (config_read_16(base_address + MM_LIMB_PROTECTION_COXA_MIN_ANGLE_OFFSET,  (uint16_t*)&g_limbs[i].coxa.prot_min_angle)  == false) return false;
        if (config_read_16(base_address + MM_LIMB_PROTECTION_COXA_MAX_ANGLE_OFFSET,  (uint16_t*)&g_limbs[i].coxa.prot_max_angle)  == false) return false;
        if (config_read_16(base_address + MM_LIMB_PROTECTION_FEMUR_MIN_ANGLE_OFFSET, (uint16_t*)&g_limbs[i].femur.prot_min_angle) == false) return false;
        if (config_read_16(base_address + MM_LIMB_PROTECTION_FEMUR_MAX_ANGLE_OFFSET, (uint16_t*)&g_limbs[i].femur.prot_max_angle) == false) return false;
        if (config_read_16(base_address + MM_LIMB_PROTECTION_TIBIA_MIN_ANGLE_OFFSET, (uint16_t*)&g_limbs[i].tibia.prot_min_angle) == false) return false;
        if (config_read_16(base_address + MM_LIMB_PROTECTION_TIBIA_MAX_ANGLE_OFFSET, (uint16_t*)&g_limbs[i].tibia.prot_max_angle) == false) return false;
        if (g_limbs[i].coxa.prot_min_angle  >= g_limbs[i].coxa.prot_max_angle  || 
            g_limbs[i].femur.prot_min_angle >= g_limbs[i].femur.prot_max_angle || 
            g_limbs[i].tibia.prot_min_angle >= g_limbs[i].tibia.prot_max_angle) {
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
/// @retval modify g_limbs
/// @return true - calculation success, false - no
//  ***************************************************************************
static bool process_linear_trajectory(float motion_time) {
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (g_current_motion.trajectories[i] != TRAJECTORY_XYZ_LINEAR) { 
            continue; // Skip limbs which not use linear trajectory
        }
      
        // Inversion motion time if need
        float relative_motion_time = motion_time;
        if (g_current_motion.time_directions[i] == TIME_DIR_REVERSE) {
            relative_motion_time = 1.0f - relative_motion_time;
        }
        
        float x0 = g_current_motion.start_positions[i].x;
        float y0 = g_current_motion.start_positions[i].y;
        float z0 = g_current_motion.start_positions[i].z;
        float x1 = g_current_motion.dest_positions[i].x;
        float y1 = g_current_motion.dest_positions[i].y;
        float z1 = g_current_motion.dest_positions[i].z;
        
        g_limbs[i].position.x = x0 + relative_motion_time * (x1 - x0);
        g_limbs[i].position.y = y0 + relative_motion_time * (y1 - y0);
        g_limbs[i].position.z = z0 + relative_motion_time * (z1 - z0);
    }
    return true;
}

//  ***************************************************************************
/// @brief  Process advanced trajectory
/// @param  motion_time: current motion time [0; 1]
/// @retval modify g_limbs
/// @return true - calculation success, false - no
//  ***************************************************************************
static bool process_advanced_trajectory(float motion_time) {
    // Check need process advanced trajectory
    // TODO: need remote it!
    bool is_need_process = false;
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (g_current_motion.trajectories[i] == TRAJECTORY_XZ_ADV_Y_CONST || 
            g_current_motion.trajectories[i] == TRAJECTORY_XZ_ADV_Y_SINUS) {
            is_need_process = true;
            break;
        }
    }
    if (is_need_process == false) {
        return true;
    }
    
    // Check curvature value
    float curvature = g_current_motion_config.curvature;
    if ((int32_t)curvature == 0)         curvature = +0.001f;
    else if ((int32_t)curvature > 1999)  curvature = +1.999f;
    else if ((int32_t)curvature < -1999) curvature = -1.999f;
    else                                 curvature = curvature / 1000.0f;
    
    // Calculation radius of curvature
    float distance = (float)g_current_motion_config.distance;
    float curvature_radius = tanf((2.0f - curvature) * M_PI / 4.0f) * fabs(distance);

    // Common calculations
    float trajectory_radius[SUPPORT_LIMBS_COUNT] = {0};
    float start_angle_rad[SUPPORT_LIMBS_COUNT] = {0};
    float max_trajectory_radius = 0;
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        
        // Skip limbs which not use advanced trajectory
        if (g_current_motion.trajectories[i] != TRAJECTORY_XZ_ADV_Y_CONST && 
            g_current_motion.trajectories[i] != TRAJECTORY_XZ_ADV_Y_SINUS) {
            continue;
        }

        // Save start point to separate variables for short code
        float x0 = g_current_motion.start_positions[i].x;
        float z0 = g_current_motion.start_positions[i].z;

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
        if (g_current_motion.trajectories[i] != TRAJECTORY_XZ_ADV_Y_CONST && 
            g_current_motion.trajectories[i] != TRAJECTORY_XZ_ADV_Y_SINUS) {
            continue;
        }
        
        // Inversion motion time if need
        float relative_motion_time = motion_time;
        if (g_current_motion.time_directions[i] == TIME_DIR_REVERSE) {
            relative_motion_time = 1.0f - relative_motion_time;
        }
        
        // Calculation arc angle for current time
        float arc_angle_rad = (relative_motion_time - 0.5f) * max_arc_angle + start_angle_rad[i];

        // Calculation XZ points by time
        g_limbs[i].position.x = curvature_radius + trajectory_radius[i] * cosf(arc_angle_rad);
        g_limbs[i].position.z =                    trajectory_radius[i] * sinf(arc_angle_rad);
        
        // Calculation Y points by time
        if (g_current_motion.trajectories[i] == TRAJECTORY_XZ_ADV_Y_CONST) {
            g_limbs[i].position.y = g_current_motion.start_positions[i].y + 0;
        } else { // TRAJECTORY_XZ_ADV_Y_SINUS
            g_limbs[i].position.y = g_current_motion.start_positions[i].y + LIMB_STEP_HEIGHT * sinf(relative_motion_time * M_PI);  
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
        float coxa_zero_rotate_deg  = g_limbs[i].coxa.zero_rotate;
        float femur_zero_rotate_deg = g_limbs[i].femur.zero_rotate;
        float tibia_zero_rotate_deg = g_limbs[i].tibia.zero_rotate;
        float coxa_length  = g_limbs[i].coxa.length;
        float femur_length = g_limbs[i].femur.length;
        float tibia_length = g_limbs[i].tibia.length;

        float x = g_limbs[i].position.x;
        float y = g_limbs[i].position.y;
        float z = g_limbs[i].position.z;


        // Move to (X*, Y*, Z*) coordinate system - rotate
        float coxa_zero_rotate_rad = DEG_TO_RAD(coxa_zero_rotate_deg);
        float x1 = x * cosf(coxa_zero_rotate_rad) + z * sinf(coxa_zero_rotate_rad);
        float y1 = y;
        float z1 = -x * sinf(coxa_zero_rotate_rad) + z * cosf(coxa_zero_rotate_rad);


        //
        // Calculate COXA angle
        //
        float coxa_angle_rad = atan2f(z1, x1);
        g_limbs[i].coxa.angle = RAD_TO_DEG(coxa_angle_rad);


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
        g_limbs[i].femur.angle = femur_zero_rotate_deg - RAD_TO_DEG(alpha) - RAD_TO_DEG(fi);
        g_limbs[i].tibia.angle = RAD_TO_DEG(gamma) - tibia_zero_rotate_deg;
        
        //
        // Protection
        //
        if (g_limbs[i].coxa.angle < g_limbs[i].coxa.prot_min_angle)   g_limbs[i].coxa.angle  = g_limbs[i].coxa.prot_min_angle;
        if (g_limbs[i].coxa.angle > g_limbs[i].coxa.prot_max_angle)   g_limbs[i].coxa.angle  = g_limbs[i].coxa.prot_max_angle;
        if (g_limbs[i].femur.angle < g_limbs[i].femur.prot_min_angle) g_limbs[i].femur.angle = g_limbs[i].femur.prot_min_angle;
        if (g_limbs[i].femur.angle > g_limbs[i].femur.prot_max_angle) g_limbs[i].femur.angle = g_limbs[i].femur.prot_max_angle;
        if (g_limbs[i].tibia.angle < g_limbs[i].tibia.prot_min_angle) g_limbs[i].tibia.angle = g_limbs[i].tibia.prot_min_angle;
        if (g_limbs[i].tibia.angle > g_limbs[i].tibia.prot_max_angle) g_limbs[i].tibia.angle = g_limbs[i].tibia.prot_max_angle;
    }
    return true;
}
#undef M_PI
#undef RAD_TO_DEG
#undef DEG_TO_RAD
