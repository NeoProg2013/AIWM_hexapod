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
#define M_PI                                (3.14159265f)
#define RAD_TO_DEG(rad)                     ((rad) * 180.0f / M_PI)
#define DEG_TO_RAD(deg)                     ((deg) * M_PI / 180.0f)


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
static bool kinematic_calculate_angles(float* y_offsets);
static void ground_level_compensation(float x_rotate, float z_rotate, float* offsets);

CLI_CMD_HANDLER(motion_cli_cmd_help);
CLI_CMD_HANDLER(motion_cli_cmd_logging);
CLI_CMD_HANDLER(motion_cli_cmd_gl_logging);
CLI_CMD_HANDLER(motion_cli_cmd_gl);

static const cli_cmd_t cli_cmd_list[] = {
    { .cmd = "help",           .handler = motion_cli_cmd_help           },
    { .cmd = "logging",        .handler = motion_cli_cmd_logging        },
    { .cmd = "gl-logging",     .handler = motion_cli_cmd_gl_logging     },
    { .cmd = "gl",             .handler = motion_cli_cmd_gl             }
};


static limb_t g_limbs[SUPPORT_LIMBS_COUNT] = {0};

static motion_t g_current_motion = {0};
static motion_config_t g_current_motion_config = {0};
static motion_config_t g_next_motion_config = {0};
static bool is_motion_completed = true;
static bool is_ground_leveling = false;

static bool is_enable_data_logging = false;
static bool is_enable_motion_data_logging = false;
static bool is_ground_leveling_logging = false;
static bool is_ground_leveling_enabled = false;



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
    float y_offsets[SUPPORT_LIMBS_COUNT] = {0};
    if (kinematic_calculate_angles(y_offsets) == false) {
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
void motion_core_start_motion(const motion_t* motion, const motion_config_t* motion_config) {
    g_current_motion = *motion;
    is_motion_completed = false;
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (g_current_motion.trajectories[i] == TRAJECTORY_XYZ_LINEAR) { // Set start point for linear traertory
            g_current_motion.start_positions[i] = g_limbs[i].position;
        }
    }
    
    g_next_motion_config = *motion_config;
    g_current_motion_config = *motion_config;
    
    if (g_current_motion.speed == 0) {
        servo_driver_set_speed(motion_config->speed);
    } else {
        servo_driver_set_speed(g_current_motion.speed);
    }
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
    
    // Gathering sensor data
    int16_t* foot_sensors_data = NULL;
    int32_t* accel_sensor_data = NULL;
    smcu_get_sensor_data(&foot_sensors_data, &accel_sensor_data);
    
    
    is_motion_completed = g_current_motion.motion_time >= g_current_motion.time_stop;
    if (!is_motion_completed) {
        
        // Scale time to [0.0; 1.0] range
        float scaled_motion_time = (float)g_current_motion.motion_time / (float)MTIME_SCALE;
        
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
        
        // Time shift and load new trajectory configuration if need
        g_current_motion.motion_time += MTIME_STEP;
        if (g_current_motion.motion_time == g_current_motion.time_update) {
            g_current_motion_config = g_next_motion_config;
            if (g_current_motion.speed == 0) {
                servo_driver_set_speed(g_current_motion_config.speed);
            } else {
                servo_driver_set_speed(g_current_motion.speed);
            }
        }
    }
    
    // Ground level compensation
    float y_offsets[SUPPORT_LIMBS_COUNT] = {0};
    if (is_ground_leveling_enabled && is_ground_leveling) {
        ground_level_compensation(accel_sensor_data[1] / 10000.0f, accel_sensor_data[0] / 10000.0f, y_offsets);
    }
    
    // Calculate servo logic angles
    if (kinematic_calculate_angles(y_offsets) == false) {
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
    
    // Logging
    if (is_enable_motion_data_logging && !is_motion_completed || is_enable_data_logging) {
        sprintf(cli_get_tx_buffer(), "[MOTION CORE]: %d %lu %d,%d,%d,%d, %d,%d,%d,%d, %d,%d,%d,%d, %d,%d,%d,%d, %d,%d,%d,%d, %d,%d,%d,%d, %d,%d\r\n", 
                (uint32_t)is_motion_completed, (uint32_t)get_time_ms(),
                (int16_t)(g_limbs[0].position.x * 100.0f), (int16_t)(g_limbs[0].position.y * 100.0f), (int16_t)(g_limbs[0].position.z * 100.0f), foot_sensors_data[0], 
                (int16_t)(g_limbs[1].position.x * 100.0f), (int16_t)(g_limbs[1].position.y * 100.0f), (int16_t)(g_limbs[1].position.z * 100.0f), foot_sensors_data[1], 
                (int16_t)(g_limbs[2].position.x * 100.0f), (int16_t)(g_limbs[2].position.y * 100.0f), (int16_t)(g_limbs[2].position.z * 100.0f), foot_sensors_data[2], 
                (int16_t)(g_limbs[3].position.x * 100.0f), (int16_t)(g_limbs[3].position.y * 100.0f), (int16_t)(g_limbs[3].position.z * 100.0f), foot_sensors_data[3], 
                (int16_t)(g_limbs[4].position.x * 100.0f), (int16_t)(g_limbs[4].position.y * 100.0f), (int16_t)(g_limbs[4].position.z * 100.0f), foot_sensors_data[4], 
                (int16_t)(g_limbs[5].position.x * 100.0f), (int16_t)(g_limbs[5].position.y * 100.0f), (int16_t)(g_limbs[5].position.z * 100.0f), foot_sensors_data[5], 
                accel_sensor_data[0], accel_sensor_data[1]);
        cli_send_data(NULL);
    } 
    if (is_ground_leveling_logging) {
        sprintf(cli_get_tx_buffer(), "[MOTION CORE]: %lu [%d,%d,%d] [%d,%d,%d] [%d,%d]\r\n", (uint32_t)get_time_ms(),
                (int16_t)(y_offsets[0]), (int16_t)(y_offsets[1]), (int16_t)(y_offsets[2]),  
                (int16_t)(y_offsets[3]), (int16_t)(y_offsets[4]), (int16_t)(y_offsets[5]),  
                (int32_t)(accel_sensor_data[1]), (int32_t)(accel_sensor_data[0]));
        cli_send_data(NULL);
    }
}

//  ***************************************************************************
/// @brief  Check motion status
/// @return true - motion completed, false - motion in progress
//  ***************************************************************************
bool motion_core_is_motion_complete(void) {
    return is_motion_completed;
}

//  ***************************************************************************
/// @brief  Enable/disable ground leveling feature
/// @return is_enable: true - enable, false - disable
//  ***************************************************************************
void motion_core_set_ground_leveling_state(bool is_enable) {
    is_ground_leveling = is_enable;
}

//  ***************************************************************************
/// @brief  Get command list for CLI
/// @param  cmd_list: pointer to cmd list size
/// @return command list
//  ***************************************************************************
const cli_cmd_t* motion_get_cmd_list(uint32_t* count) {
    *count = sizeof(cli_cmd_list) / sizeof(cli_cmd_t);
    return cli_cmd_list;
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
/// @brief  Ground level compensation
/// @param  x_rotate, z_rotate: hexapod plane rotate relative ground plane
/// @return true - calculation success, false - no
//  ***************************************************************************
static void ground_level_compensation(float x_rotate, float z_rotate, float* offsets) {
    const float max_rotate_angle = 6.4f;
    if (z_rotate < -max_rotate_angle) {
        z_rotate = -max_rotate_angle;
    } else if (z_rotate > max_rotate_angle) {
        z_rotate = max_rotate_angle;
    }
    if (x_rotate < -max_rotate_angle) {
        x_rotate = -max_rotate_angle;
    } else if (x_rotate > max_rotate_angle) {
        x_rotate = max_rotate_angle;
    }

    
    // Surface normal
    point_3d_t n;
    n.x = 0;
    n.y = 1;
    n.z = 0;
    
    // Move surface to max position
    point_3d_t a = {0, 0, 0};
    if (z_rotate < 0) a.x = +(135.0f + 80.f);
    else              a.x = -(135.0f + 80.f);
    if (x_rotate < 0) a.z = +(70.0f + 104.0f);
    else              a.z = -(70.0f + 104.0f);
    
    // Rotate normal by axis X
    float x_rotate_rad = DEG_TO_RAD(x_rotate);
    n.y = n.y * cosf(x_rotate_rad) + n.z * sinf(x_rotate_rad);
    n.z = n.y * sinf(x_rotate_rad) - n.z * cosf(x_rotate_rad);
    
    // Rotate normal by axis Z
    float z_rotate_rad = DEG_TO_RAD(z_rotate);
    n.x = n.x * cosf(z_rotate_rad) + n.y * sinf(z_rotate_rad);
    n.y = n.x * sinf(z_rotate_rad) - n.y * cosf(z_rotate_rad);
    
    // Calculate Y offsets
    for (int32_t i = 0; i < sizeof(g_limbs) / sizeof(g_limbs[0]); ++i) {
        float z_sign = 1.0f;
        if (g_limbs[i].position.z != 0) {
            z_sign = g_limbs[i].position.z / fabs(g_limbs[i].position.z);
        }
        float z = g_limbs[i].position.z + z_sign * 104.0f; // 104 - coxa position from center by axix Z
        
        float x_sign = 1.0f;
        if (g_limbs[i].position.x != 0) {
            x_sign = g_limbs[i].position.x / fabs(g_limbs[i].position.x);
        }
        float x = g_limbs[i].position.x + x_sign * 104.0f;
        
        offsets[i] = (-1) * (-n.z * (z - a.z) - n.x * (x - a.x)) / n.y;
    }
}

//  ***************************************************************************
/// @brief  Calculate angles
/// @param  y_offsets: offset by Y axis for each limb
/// @retval limbs_list::servo_angle
/// @return true - calculation success, false - no
//  ***************************************************************************
static bool kinematic_calculate_angles(float* y_offsets) {
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        float coxa_zero_rotate_deg  = g_limbs[i].coxa.zero_rotate;
        float femur_zero_rotate_deg = g_limbs[i].femur.zero_rotate;
        float tibia_zero_rotate_deg = g_limbs[i].tibia.zero_rotate;
        float coxa_length  = g_limbs[i].coxa.length;
        float femur_length = g_limbs[i].femur.length;
        float tibia_length = g_limbs[i].tibia.length;

        float x = g_limbs[i].position.x;
        float y = g_limbs[i].position.y + y_offsets[i];
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
            void* tx_buffer = cli_get_tx_buffer();
            sprintf(tx_buffer, "[MOTION CORE]: Point not attainable\r\n");
            cli_send_data(NULL);
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
        if (g_limbs[i].coxa.angle < g_limbs[i].coxa.prot_min_angle) {
            sprintf(cli_get_tx_buffer(), "[MOTION CORE]: COXA protection from %d to %d\r\n", (int32_t)g_limbs[i].coxa.angle, g_limbs[i].coxa.prot_min_angle);
            cli_send_data(NULL);
            g_limbs[i].coxa.angle = g_limbs[i].coxa.prot_min_angle;
        }
        if (g_limbs[i].coxa.angle > g_limbs[i].coxa.prot_max_angle) {
            sprintf(cli_get_tx_buffer(), "[MOTION CORE]: COXA protection from %d to %d\r\n", (int32_t)g_limbs[i].coxa.angle, g_limbs[i].coxa.prot_max_angle);
            cli_send_data(NULL);
            g_limbs[i].coxa.angle = g_limbs[i].coxa.prot_max_angle;
        }
        if (g_limbs[i].femur.angle < g_limbs[i].femur.prot_min_angle) {
            sprintf(cli_get_tx_buffer(), "[MOTION CORE]: FEMUR protection from %d to %d\r\n", (int32_t)g_limbs[i].femur.angle, g_limbs[i].femur.prot_min_angle);
            cli_send_data(NULL);
            g_limbs[i].femur.angle = g_limbs[i].femur.prot_min_angle;
        }
        if (g_limbs[i].femur.angle > g_limbs[i].femur.prot_max_angle) {
            sprintf(cli_get_tx_buffer(), "[MOTION CORE]: FEMUR protection from %d to %d\r\n", (int32_t)g_limbs[i].femur.angle, g_limbs[i].femur.prot_max_angle);
            cli_send_data(NULL);
            g_limbs[i].femur.angle = g_limbs[i].femur.prot_max_angle;
        }
        if (g_limbs[i].tibia.angle < g_limbs[i].tibia.prot_min_angle) {
            sprintf(cli_get_tx_buffer(), "[MOTION CORE]: TIBIA protection from %d to %d\r\n", (int32_t)g_limbs[i].tibia.angle, g_limbs[i].tibia.prot_min_angle);
            cli_send_data(NULL);
            g_limbs[i].tibia.angle = g_limbs[i].tibia.prot_min_angle;
        }
        if (g_limbs[i].tibia.angle > g_limbs[i].tibia.prot_max_angle) {
            sprintf(cli_get_tx_buffer(), "[MOTION CORE]: TIBIA protection from %d to %d\r\n", (int32_t)g_limbs[i].tibia.angle, g_limbs[i].tibia.prot_max_angle);
            cli_send_data(NULL);
            g_limbs[i].tibia.angle = g_limbs[i].tibia.prot_max_angle;
        }
    }
    return true;
}





// ***************************************************************************
// CLI SECTION
// ***************************************************************************
CLI_CMD_HANDLER(motion_cli_cmd_help) {
    const char* help = CLI_HELP(
        "[MOTION SUBSYSTEM]\r\n"
        "Commands: \r\n"
        "  motion logging <0|1> [m] - enable data logging, m - for motion\r\n"
        "  motion gl-logging <0|1> - enable logging for ground leveling\r\n"
        "  motion gl <0|1> - enable ground leveling feature");
    if (strlen(response) >= USART1_TX_BUFFER_SIZE) {
        strcpy(response, CLI_ERROR("Help message size more USART1_TX_BUFFER_SIZE"));
        return false;
    }
    strcpy(response, help);
    return true;
}
CLI_CMD_HANDLER(motion_cli_cmd_logging) {
    if (argc < 1) {
        strcpy(response, CLI_ERROR("Bad usage. Use \"motion help\" for details"));
        return false;
    }
    
    if (argv[0][0] == '1') {
        is_enable_data_logging = true;
        if (argc >= 2 && argv[1][0] == 'm') {
            is_enable_motion_data_logging = true;
            is_enable_data_logging = false;
        }
    } else {
        is_enable_data_logging = false;
        is_enable_motion_data_logging = false;
    }
    return true;
}
CLI_CMD_HANDLER(motion_cli_cmd_gl_logging) {
    if (argc != 1) {
        strcpy(response, CLI_ERROR("Bad usage. Use \"motion help\" for details"));
        return false;
    }
    is_ground_leveling_logging = (argv[0][0] == '1');
    return true;
}
CLI_CMD_HANDLER(motion_cli_cmd_gl) {
    if (argc != 1) {
        strcpy(response, CLI_ERROR("Bad usage. Use \"motion help\" for details"));
        return false;
    }
    is_ground_leveling_enabled = (argv[0][0] == '1');
    return true;
}
