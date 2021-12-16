/// ***************************************************************************
/// @file    motion_core.c
/// @author  NeoProg
/// ***************************************************************************
#include "project_base.h"
#include "motion_core.h"
#include "motion_math.h"
#include "servo_driver.h"
#include "configurator.h"
#include "systimer.h"
#include "pwm.h"
#include "system_monitor.h"
#include <math.h>
#include <float.h>
#define CHANGE_HEIGHT_MAX_STEP                  (0.5f)
#define HEXAPOD_UP_HEIGHT_THRESHOLD             (-85)
#define MOTION_STEP_HEIGHT                      (30)

#define MOTION_LIMBS_DOWN_TIMEOUT               (300)

#define MOTION_TIME_MIN_VALUE                   (0)
#define MOTION_TIME_MID_VALUE                   (500)
#define MOTION_TIME_MAX_VALUE                   (1000)


static bool load_config(void);



static const v3d_t limbs_base_pos[] = {
        {-115, 0, 70}, {-135, 0, 0}, {-115, 0, -70}, // Left side
        { 115, 0, 70}, { 135, 0, 0}, { 115, 0, -70}  // Right side
};
static const int16_t surface_down_y = -15;
static const int16_t surface_up_y = -85;

static const int16_t limbs_up_y = -55;


static limb_t g_limbs[SUPPORT_LIMBS_COUNT] = {0};
static motion_t g_cur_motion = {0};
static motion_t g_dst_motion = {0};


typedef enum {
    HEXAPOD_STATE_DOWN,
    HEXAPOD_STATE_RDY,
    HEXAPOD_STATE_INIT,
    HEXAPOD_STATE_MOVE,
    HEXAPOD_STATE_DEINIT
} hexapod_state_t;

hexapod_state_t hexapod_state = HEXAPOD_STATE_RDY;//HEXAPOD_STATE_DOWN;


/// ***************************************************************************
/// @brief  Motion core initialization
/// @param  start_points: limbs start points list
/// ***************************************************************************
void motion_core_init() {
    // Initialize servo driver
    servo_driver_init();

    // Load motion core configuration
    if (!load_config()) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
        return;
    }
    
    // Load start points
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        g_limbs[i].pos = limbs_base_pos[i];
    }

    // Make motion surface
    g_cur_motion.surface_point.x = 0;
    g_cur_motion.surface_point.y = surface_down_y;
    g_cur_motion.surface_point.z = 0;
    g_dst_motion = g_cur_motion;

    // Calculate limbs offset by regarding surface
    if (!mm_surface_calculate_offsets(g_limbs, &g_cur_motion.surface_point, &g_cur_motion.surface_rotate)) {
        sysmon_set_error(SYSMON_MATH_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
        return;
    }

    // Set servo start angles
    if (!mm_kinematic_calculate_angles(g_limbs)) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
        return;
    }

    // Load start angles and turn servo power ON
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        servo_driver_move(i * 3 + 0, g_limbs[i].coxa.angle);
        servo_driver_move(i * 3 + 1, g_limbs[i].femur.angle);
        servo_driver_move(i * 3 + 2, g_limbs[i].tibia.angle);
    }
    servo_driver_power_on();
}

/// ***************************************************************************
/// @brief  Start motion
/// @param  motion: motion description. @ref motion_t
/// ***************************************************************************
void motion_core_get_current_motion(motion_t* motion) {
    *motion = g_cur_motion;
}
void motion_core_start_motion(const motion_t* motion) {
    g_dst_motion = *motion;
    /*if (hexapod_state == HEXAPOD_STATE_DOWN) {
        g_dst_motion.curvature = 0;
        g_dst_motion.distance = 0;
        g_dst_motion.speed = 0;
        g_dst_motion.surface_rotate.x = 0;
        g_dst_motion.surface_rotate.y = 0;
        g_dst_motion.surface_rotate.z = 0;
        if (g_dst_motion.surface_point.y > surface_up_y) {
            g_dst_motion.surface_point = surface_down_y;
        }
    }*/
}

void motion_core_stop_motion(void) {

}

void motion_core_update_motion(const motion_t* motion) {

}

/// ***************************************************************************
/// @brief  Motion core process
/// @note   Call each PWM period
/// ***************************************************************************
void motion_core_process(void) {
    if (sysmon_is_module_disable(SYSMON_MODULE_MOTION_DRIVER) == true) return;  // Module disabled

    //
    // Main motion process
    //
    static int32_t motion_time = MOTION_TIME_MIN_VALUE;
    static int32_t motion_loop = 0;
    static uint64_t start_time = 0;
    if (hexapod_state == HEXAPOD_STATE_RDY) {
        if (g_dst_motion.distance) {
            hexapod_state = HEXAPOD_STATE_INIT;
        } else if (get_time_ms() - start_time > MOTION_LIMBS_DOWN_TIMEOUT) {
            hexapod_state = HEXAPOD_STATE_DEINIT;
        }
    } else if (hexapod_state == HEXAPOD_STATE_INIT) {
        bool is_completed = true;
        // Move limbs 0, 2, 4 to up state for even loop
        // Move limbs 1, 3, 5 to up state for odd loop
        for (int32_t i = motion_loop & 0x01; i < SUPPORT_LIMBS_COUNT; i += 2) { 
            float diff = mm_calc_step(g_limbs[i].pos.y, MOTION_STEP_HEIGHT, CHANGE_HEIGHT_MAX_STEP);
            if (fabs(diff) > FLT_EPSILON) {
                is_completed = false;
            }
            g_limbs[i].pos.y += diff;
        }
        if (is_completed) {
            motion_time = MOTION_TIME_MID_VALUE;
            hexapod_state = HEXAPOD_STATE_MOVE;
        }
    } else if (hexapod_state == HEXAPOD_STATE_MOVE) {
        if (motion_time == MOTION_TIME_MID_VALUE) {
            g_cur_motion.curvature = g_dst_motion.curvature;
            g_cur_motion.distance  = g_dst_motion.distance;
            g_cur_motion.speed     = g_dst_motion.speed;
        }
        if (g_cur_motion.distance) {
            if (!mm_process_advanced_traj(g_limbs, limbs_base_pos, motion_time / 1000.0f, motion_loop, g_cur_motion.curvature, g_cur_motion.distance, MOTION_STEP_HEIGHT)) {
                sysmon_set_error(SYSMON_MATH_ERROR);
                sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
                return;
            }
            motion_time += 5;
            if (motion_time > MOTION_TIME_MAX_VALUE) {
                motion_time = MOTION_TIME_MIN_VALUE;
                ++motion_loop;
            }
        } else {
            hexapod_state = HEXAPOD_STATE_RDY;
            start_time = get_time_ms();
        }
    } else if (hexapod_state == HEXAPOD_STATE_DEINIT) {
        if (g_dst_motion.distance == 0) {
            bool is_completed = true;
            // Move all limbs to down state
            for (int32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) { 
                float diff = mm_calc_step(g_limbs[i].pos.y, 0, CHANGE_HEIGHT_MAX_STEP);
                if (fabs(diff) > FLT_EPSILON) {
                    is_completed = false;
                }
                g_limbs[i].pos.y += diff;
            }
            if (is_completed) {
                motion_time = MOTION_TIME_MIN_VALUE;
                motion_loop = 0;
                hexapod_state = HEXAPOD_STATE_RDY;
            }
        } else {
            hexapod_state = HEXAPOD_STATE_RDY;
        }
    }
    
    // Change height process
    g_cur_motion.surface_point.y += mm_calc_step(g_cur_motion.surface_point.y, g_dst_motion.surface_point.y, CHANGE_HEIGHT_MAX_STEP);

    // Calculate limbs offset by relatively surface
    if (!mm_surface_calculate_offsets(g_limbs, &g_cur_motion.surface_point, &g_cur_motion.surface_rotate)) {
        sysmon_set_error(SYSMON_MATH_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
        return;
    }

    // Calculate servo logic angles
    if (mm_kinematic_calculate_angles(g_limbs) == false) {
        sysmon_set_error(SYSMON_MATH_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
        return;
    }

    // Load new angles to servo driver
    for (int32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        servo_driver_move(i * 3 + 0, g_limbs[i].coxa.angle);
        servo_driver_move(i * 3 + 1, g_limbs[i].femur.angle);
        servo_driver_move(i * 3 + 2, g_limbs[i].tibia.angle);
    }


    /*switch (hexapod_state) {
        case HEXAPOD_STATE_DOWN:
            break;
        case HEXAPOD_STATE_RDY:
            break;
        case HEXAPOD_STATE_PREPARE:
            break;
        case HEXAPOD_STATE_MOVE:
            break;
    }*/

    // Gathering sensor data
    //int16_t* foot_sensors_data = NULL;
    //int32_t* accel_sensor_data = NULL;
    //smcu_get_sensor_data(&foot_sensors_data, &accel_sensor_data);
    
    
    /*is_motion_completed = g_current_motion.motion_time >= g_current_motion.time_stop;
    if (!is_motion_completed) {
        
        // Scale time to [0.0; 1.0] range
        float scaled_motion_time = (float)g_current_motion.motion_time / (float)MTIME_SCALE;
        
        // Calculate new limbs positions
        if (!process_linear_traj(scaled_motion_time) || !process_advanced_traj(scaled_motion_time)) {
            sysmon_set_error(SYSMON_MATH_ERROR);
            sysmon_disable_module(SYSMON_MODULE_MOTION_DRIVER);
            return;
        }
        
        // Time shift and load new trajectory configuration if need
        g_current_motion.motion_time += MTIME_STEP;
        if (g_current_motion.motion_time == g_current_motion.time_update) {
            g_current_user_motion_cfg = g_next_user_motion_cfg;
            if (g_current_motion.speed == 0) {
                servo_driver_set_speed(g_current_user_motion_cfg.speed);
            } else {
                servo_driver_set_speed(g_current_motion.speed);
            }
        }
    }
    
    // Ground level compensation
    float y_offsets[LIMBS_COUNT] = {0};
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
    for (uint32_t i = 0; i < LIMBS_COUNT; ++i) {
        servo_driver_move(i * 3 + 0, g_limbs[i].coxa.angle);
        servo_driver_move(i * 3 + 1, g_limbs[i].femur.angle);
        servo_driver_move(i * 3 + 2, g_limbs[i].tibia.angle);
    }
    
    // Logging
    if (is_enable_motion_data_logging && !is_motion_completed || is_enable_data_logging) {
        sprintf(cli_get_tx_buffer(), "[MOTION CORE]: %d %lu %d,%d,%d,%d, %d,%d,%d,%d, %d,%d,%d,%d, %d,%d,%d,%d, %d,%d,%d,%d, %d,%d,%d,%d, %d,%d\r\n", 
                (uint32_t)is_motion_completed, (uint32_t)get_time_ms(),
                (int16_t)(g_limbs[0].pos.x * 100.0f), (int16_t)(g_limbs[0].pos.y * 100.0f), (int16_t)(g_limbs[0].pos.z * 100.0f), foot_sensors_data[0], 
                (int16_t)(g_limbs[1].pos.x * 100.0f), (int16_t)(g_limbs[1].pos.y * 100.0f), (int16_t)(g_limbs[1].pos.z * 100.0f), foot_sensors_data[1], 
                (int16_t)(g_limbs[2].pos.x * 100.0f), (int16_t)(g_limbs[2].pos.y * 100.0f), (int16_t)(g_limbs[2].pos.z * 100.0f), foot_sensors_data[2], 
                (int16_t)(g_limbs[3].pos.x * 100.0f), (int16_t)(g_limbs[3].pos.y * 100.0f), (int16_t)(g_limbs[3].pos.z * 100.0f), foot_sensors_data[3], 
                (int16_t)(g_limbs[4].pos.x * 100.0f), (int16_t)(g_limbs[4].pos.y * 100.0f), (int16_t)(g_limbs[4].pos.z * 100.0f), foot_sensors_data[4], 
                (int16_t)(g_limbs[5].pos.x * 100.0f), (int16_t)(g_limbs[5].pos.y * 100.0f), (int16_t)(g_limbs[5].pos.z * 100.0f), foot_sensors_data[5], 
                accel_sensor_data[0], accel_sensor_data[1]);
        cli_send_data(NULL);
    } 
    if (is_ground_leveling_logging) {
        sprintf(cli_get_tx_buffer(), "[MOTION CORE]: %lu [%d,%d,%d] [%d,%d,%d] [%d,%d]\r\n", (uint32_t)get_time_ms(),
                (int16_t)(y_offsets[0]), (int16_t)(y_offsets[1]), (int16_t)(y_offsets[2]),  
                (int16_t)(y_offsets[3]), (int16_t)(y_offsets[4]), (int16_t)(y_offsets[5]),  
                (int32_t)(accel_sensor_data[1]), (int32_t)(accel_sensor_data[0]));
        cli_send_data(NULL);
    }*/
}




        
/// ***************************************************************************
/// @brief  Load configuration
/// @return true - load and validate success, false - fail
/// ***************************************************************************
static bool load_config(void) {
    uint32_t base_address = MM_LIMB_CONFIG_BASE_EE_ADDRESS;

    // Read length
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (!config_read_16(base_address + MM_LIMB_COXA_LENGTH_OFFSET,  &g_limbs[i].coxa.length))  return false;
        if (!config_read_16(base_address + MM_LIMB_FEMUR_LENGTH_OFFSET, &g_limbs[i].femur.length)) return false;
        if (!config_read_16(base_address + MM_LIMB_TIBIA_LENGTH_OFFSET, &g_limbs[i].tibia.length)) return false;
    }
    
    // Read zero rotate
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (!config_read_16(base_address + MM_LIMB_COXA_ZERO_ROTATE_OFFSET + i * sizeof(uint16_t), (uint16_t*)&g_limbs[i].coxa.zero_rotate)) return false;
        if (!config_read_16(base_address + MM_LIMB_FEMUR_ZERO_ROTATE_OFFSET, (uint16_t*)&g_limbs[i].femur.zero_rotate)) return false;
        if (!config_read_16(base_address + MM_LIMB_TIBIA_ZERO_ROTATE_OFFSET, (uint16_t*)&g_limbs[i].tibia.zero_rotate)) return false;
        if (abs(g_limbs[i].coxa.zero_rotate) > 360 || abs(g_limbs[i].femur.zero_rotate) > 360 || abs(g_limbs[i].tibia.zero_rotate) > 360) {
            return false;
        }
    }
    
    // Read protection
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        if (!config_read_16(base_address + MM_LIMB_PROTECTION_COXA_MIN_ANGLE_OFFSET,  (uint16_t*)&g_limbs[i].coxa.prot_min_angle))  return false;
        if (!config_read_16(base_address + MM_LIMB_PROTECTION_COXA_MAX_ANGLE_OFFSET,  (uint16_t*)&g_limbs[i].coxa.prot_max_angle))  return false;
        if (!config_read_16(base_address + MM_LIMB_PROTECTION_FEMUR_MIN_ANGLE_OFFSET, (uint16_t*)&g_limbs[i].femur.prot_min_angle)) return false;
        if (!config_read_16(base_address + MM_LIMB_PROTECTION_FEMUR_MAX_ANGLE_OFFSET, (uint16_t*)&g_limbs[i].femur.prot_max_angle)) return false;
        if (!config_read_16(base_address + MM_LIMB_PROTECTION_TIBIA_MIN_ANGLE_OFFSET, (uint16_t*)&g_limbs[i].tibia.prot_min_angle)) return false;
        if (!config_read_16(base_address + MM_LIMB_PROTECTION_TIBIA_MAX_ANGLE_OFFSET, (uint16_t*)&g_limbs[i].tibia.prot_max_angle)) return false;
        if (g_limbs[i].coxa.prot_min_angle  >= g_limbs[i].coxa.prot_max_angle  || 
            g_limbs[i].femur.prot_min_angle >= g_limbs[i].femur.prot_max_angle || 
            g_limbs[i].tibia.prot_min_angle >= g_limbs[i].tibia.prot_max_angle) {
            return false;
        }
    }
    return true;
}

/// ***************************************************************************
/// @brief  Process linear trajectory
/// @param  motion_time: current motion time [0; 1]
/// @retval modify g_limbs
/// @return true - calculation success, false - no
/// ***************************************************************************
/*static bool process_linear_traj(float motion_time) {
    for (uint32_t i = 0; i < LIMBS_COUNT; ++i) {
        if (g_current_motion.traj[i] != TRAJ_XYZ_LINEAR) { 
            continue; // Skip limbs which not use linear trajectory
        }
      
        // Inversion motion time if need
        float relative_motion_time = motion_time;
        if (g_current_motion.time_dir[i] == TIME_DIR_REVERSE) {
            relative_motion_time = 1.0f - relative_motion_time;
        }
        
        float x0 = g_current_motion.start_pos[i].x;
        float y0 = g_current_motion.start_pos[i].y;
        float z0 = g_current_motion.start_pos[i].z;
        float x1 = g_current_motion.dst_pos[i].x;
        float y1 = g_current_motion.dst_pos[i].y;
        float z1 = g_current_motion.dst_pos[i].z;
        
        g_limbs[i].pos.x = x0 + relative_motion_time * (x1 - x0);
        g_limbs[i].pos.y = y0 + relative_motion_time * (y1 - y0);
        g_limbs[i].pos.z = z0 + relative_motion_time * (z1 - z0);
    }
    return true;
}*/









// ***************************************************************************
// CLI SECTION
// ***************************************************************************
/*CLI_CMD_HANDLER(motion_cli_cmd_help) {
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
}*/
