/// ***************************************************************************
/// @file    motion-core.c
/// @author  NeoProg
/// ***************************************************************************
#include "project-base.h"
#include "motion-core.h"
#include "motion-math.h"
#include "servo-driver.h"
#include "sensors-core.h"
#include "pwm.h"
#include "system-monitor.h"
#include "pca9555.h"
#define CHANGE_SURFACE_POS_MAX_STEP             (1.5f)

#define MOTION_MIN_STEP_HEIGHT                  (15)
#define MOTION_MAX_STEP_HEIGHT                  (60)
#define MOTION_DEFAULT_STEP_HEIGHT              (30)

#define MOTION_SURFACE_MIN_HEIGHT               (-15)
#define MOTION_SURFACE_MAX_HEIGHT               (-150)
#define MOTION_SURFACE_UP_HEIGHT_THRESHOLD      (-85)

#define MOTION_LIMBS_DOWN_TIMEOUT               (100)

#define MOTION_TIME_MIN_VALUE                   (0)
#define MOTION_TIME_MID_VALUE                   (500)
#define MOTION_TIME_MAX_VALUE                   (1000)
#define MOTION_TIME_STEP                        (20)


typedef enum {
    HEXAPOD_STATE_DOWN,
    HEXAPOD_STATE_RDY,
    HEXAPOD_STATE_MOTION_INIT,
    HEXAPOD_STATE_MOTION_EXEC,
    HEXAPOD_STATE_MOTION_DEINIT
} g_hexapod_state_t;

typedef struct {
    motion_cfg_t cfg;
    p3d_t surface_point;
    r3d_t surface_rotate;
} motion_t;


static void load_config(void);
static void main_motion_process(void);


static const v3d_t g_limbs_base_pos[] = {
    {-115, 0, 70}, {-135, 0, 0}, {-115, 0, -70}, // Left side
    { 115, 0, 70}, { 135, 0, 0}, { 115, 0, -70}  // Right side
};
static limb_t g_limbs[SUPPORT_LIMBS_COUNT] = {0};
static motion_t g_cur_motion = {0};
static ext_motion_t g_ext_motion = {0};
static g_hexapod_state_t g_hexapod_state = HEXAPOD_STATE_DOWN;
static bool g_is_surface_move_completed = false;



/// ***************************************************************************
/// @brief  Motion core initialization
/// ***************************************************************************
void motion_core_init() {
    load_config();
    
    // Load start points
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        g_limbs[i].pos = g_limbs_base_pos[i];
    }

    // Init motion
    g_cur_motion.surface_point.y = g_ext_motion.surface_point.y = MOTION_SURFACE_MIN_HEIGHT;
    g_cur_motion.cfg.step_height = g_ext_motion.cfg.step_height = MOTION_DEFAULT_STEP_HEIGHT;

    /*// Calculate limbs offset by regarding surface
    if (!mm_surface_calculate_offsets(g_limbs, &g_cur_motion.surface_point, &g_cur_motion.surface_rotate)) {
        sysmon_set_error(SYSMON_MATH_ERROR | SYSMON_FATAL_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_CORE);
        return;
    }

    // Set servo start angles
    if (!mm_kinematic_calculate_angles(g_limbs)) {
        sysmon_set_error(SYSMON_MATH_ERROR | SYSMON_FATAL_ERROR);
        sysmon_disable_module(SYSMON_MODULE_MOTION_CORE);
        return;
    }

    // Load start angles and turn servo power ON
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        servo_driver_move(i * 3 + 0, g_limbs[i].coxa.angle);
        servo_driver_move(i * 3 + 1, g_limbs[i].femur.angle);
        servo_driver_move(i * 3 + 2, g_limbs[i].tibia.angle);
    }*/
    servo_driver_power_on();
}

/// ***************************************************************************
/// @brief  Main motion control
/// @param  ext_motion: user_motion description, may be NULL. @ref ext_motion_t
/// ***************************************************************************
void motion_core_move(const ext_motion_t* ext_motion) {
    if (!ext_motion) { 
        memset(&g_ext_motion, 0, sizeof(g_ext_motion));
        return;
    }
    g_ext_motion = *ext_motion;
    
    // Inhibit any motions if hexapod is down except change height for stand up
    if (g_hexapod_state == HEXAPOD_STATE_DOWN) {
        memset(&g_ext_motion, 0, sizeof(g_ext_motion));
        if (ext_motion->surface_point.y <= MOTION_SURFACE_UP_HEIGHT_THRESHOLD) {
            g_ext_motion.surface_point.y = ext_motion->surface_point.y;
        }
    }
}

/// ***************************************************************************
/// @brief  Get current motion parameters
/// @return Copy of motion parameters
/// ***************************************************************************
ext_motion_t motion_core_get_motion(void) {
    ext_motion_t ext_motion = {0};
    ext_motion.cfg = g_cur_motion.cfg;
    ext_motion.surface_point = g_cur_motion.surface_point;
    ext_motion.surface_rotate = g_cur_motion.surface_rotate;
    return ext_motion;
}

/// ***************************************************************************
/// @brief  Motion core process
/// @note   Call each PWM period from main loop
/// ***************************************************************************
extern uint16_t sensors_inputs;
void motion_core_process(void) {
    if (sysmon_is_module_disable(SYSMON_MODULE_MOTION_CORE)) return;  // Module disabled
    sysmon_clear_error(SYSMON_MATH_ERROR);
    
    //
    // Motion section
    //
    // Constrain step height before motions
    constrain_u16(&g_ext_motion.cfg.step_height, MOTION_MIN_STEP_HEIGHT, MOTION_MAX_STEP_HEIGHT); 
    
    // Change motion speed 
    servo_driver_set_speed(g_ext_motion.cfg.speed);

    // Motion iteration process
    main_motion_process();
    
    //
    // Make result surface
    //
    // Apply external surface
    v3d_t dst_surface_point  = g_ext_motion.surface_point;
    r3d_t dst_surface_rotate = g_ext_motion.surface_rotate;
    
    // Apply hull rotate surface
    float xz[2] = {0};
    sensors_core_get_orientation(xz);
    if (g_ext_motion.ctrl & MOTION_CTRL_EN_STAB && g_hexapod_state != HEXAPOD_STATE_DOWN) {
        dst_surface_rotate.x -= xz[0];
        dst_surface_rotate.z -= xz[1];
    }
    
    // Contrain surface rotate
    const float max_rotate_angle = 6.4f;
    constrain_float(&dst_surface_rotate.x, -max_rotate_angle, max_rotate_angle);
    constrain_float(&dst_surface_rotate.z, -max_rotate_angle, max_rotate_angle);

    // Constrain surface Y offset. Inhibit move hexapod to down while motion is progress
    float min_y_surface_point = (g_hexapod_state == HEXAPOD_STATE_MOTION_EXEC) ? MOTION_SURFACE_UP_HEIGHT_THRESHOLD : MOTION_SURFACE_MIN_HEIGHT;
    constrain_float(&dst_surface_point.y, MOTION_SURFACE_MAX_HEIGHT, min_y_surface_point);

    //
    // Move surface
    //
    g_is_surface_move_completed = mm_move_surface(&g_cur_motion.surface_point, &dst_surface_point, &g_cur_motion.surface_rotate, &dst_surface_rotate, CHANGE_SURFACE_POS_MAX_STEP);
    
    //
    // Change hexapod state relatively reached MOTION_SURFACE_UP_HEIGHT_THRESHOLD by axis Y
    //
    if (islessequal(g_cur_motion.surface_point.y, MOTION_SURFACE_UP_HEIGHT_THRESHOLD)) { // We reach MOTION_SURFACE_UP_HEIGHT_THRESHOLD by axis Y
        if (g_is_surface_move_completed && g_hexapod_state == HEXAPOD_STATE_DOWN) {
            g_hexapod_state = HEXAPOD_STATE_RDY;
        }
    } else {
        if (g_hexapod_state == HEXAPOD_STATE_RDY) {
            g_hexapod_state = HEXAPOD_STATE_DOWN;
        }
    }

    // Calculate limbs offset relatively surface
    if (!mm_surface_calculate_offsets(g_limbs, &g_cur_motion.surface_point, &g_cur_motion.surface_rotate)) {
        sysmon_set_error(SYSMON_MATH_ERROR);
        return;
    }

    // Calculate servo logic angles
    if (!mm_kinematic_calculate_angles(g_limbs)) {
        sysmon_set_error(SYSMON_MATH_ERROR);
        return;
    }

    // Load new angles to servo driver
    for (int32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        servo_driver_move(i * 3 + 0, g_limbs[i].coxa.angle);
        servo_driver_move(i * 3 + 1, g_limbs[i].femur.angle);
        servo_driver_move(i * 3 + 2, g_limbs[i].tibia.angle);
    }
    
    void* tx_buffer = cli_get_tx_buffer();
    sprintf(tx_buffer, "[MCORE]: %d sensors: %d,%d,%d %d,%d,%d  pos: %d,%d,%d,%d,%d,%d  rotate: %d,%d,%d  mpu: %d,%d\r\n", 
            (int32_t)get_time_ms(), 
            (sensors_inputs & PCA9555_GPIO_SENSOR_LEFT_1) != 0, (sensors_inputs & PCA9555_GPIO_SENSOR_LEFT_2) != 0, (sensors_inputs & PCA9555_GPIO_SENSOR_LEFT_3) != 0,
            (sensors_inputs & PCA9555_GPIO_SENSOR_RIGHT_1) != 0, (sensors_inputs & PCA9555_GPIO_SENSOR_RIGHT_2) != 0, (sensors_inputs & PCA9555_GPIO_SENSOR_RIGHT_3) != 0,
            (int32_t)(g_limbs[0].pos.y), (int32_t)(g_limbs[1].pos.y), (int32_t)(g_limbs[2].pos.y),
            (int32_t)(g_limbs[3].pos.y), (int32_t)(g_limbs[4].pos.y), (int32_t)(g_limbs[5].pos.y),
            (int32_t)g_cur_motion.surface_rotate.x, (int32_t)g_cur_motion.surface_rotate.y, (int32_t)g_cur_motion.surface_rotate.z, (int32_t)xz[0], (int32_t)xz[1]);
    cli_send_data(NULL);
}

/// ***************************************************************************
/// @brief  Get Hexapod down state
/// @return true -- hexapod is down
/// ***************************************************************************
bool motion_core_is_down(void) {
    return g_hexapod_state == HEXAPOD_STATE_DOWN;
}



/// ***************************************************************************
/// @brief  Main motion process
/// ***************************************************************************
static void main_motion_process(void) {
    static int32_t motion_time = MOTION_TIME_MIN_VALUE;
    static int32_t motion_loop = 0;

    if (g_hexapod_state == HEXAPOD_STATE_RDY && g_ext_motion.cfg.distance) {
        g_cur_motion.cfg = g_ext_motion.cfg; // Update motion configuration
        g_hexapod_state = HEXAPOD_STATE_MOTION_INIT;
    } 
    
    if (g_hexapod_state == HEXAPOD_STATE_MOTION_INIT) {
        // Move limbs 0, 2, 4 to up state for even loop
        // Move limbs 1, 3, 5 to up state for odd loop
        bool is_completed = true;
        for (int32_t i = motion_loop & 0x01; i < SUPPORT_LIMBS_COUNT; i += 2) {
            if (!mm_move_value(&g_limbs[i].pos.y, g_cur_motion.cfg.step_height, CHANGE_SURFACE_POS_MAX_STEP)) {
                is_completed = false;
            }
        }
        if (is_completed) {
            motion_time = MOTION_TIME_MID_VALUE;
            g_hexapod_state = HEXAPOD_STATE_MOTION_EXEC;
        }
    } 
    else if (g_hexapod_state == HEXAPOD_STATE_MOTION_EXEC) {
        if (motion_time == MOTION_TIME_MID_VALUE) { // Reached update motion configuration time
            g_cur_motion.cfg = g_ext_motion.cfg;
        }
        
        static uint64_t last_exec_time = 0;
        if (g_cur_motion.cfg.distance) {
            const motion_cfg_t* cfg = &g_cur_motion.cfg;
            if (!mm_process_advanced_traj(g_limbs, g_limbs_base_pos, motion_time, motion_loop, cfg->curvature, cfg->distance, cfg->step_height)) {
                sysmon_set_error(SYSMON_MATH_ERROR);
                sysmon_disable_module(SYSMON_MODULE_MOTION_CORE);
                return;
            }
            motion_time += MOTION_TIME_STEP;
            if (motion_time > MOTION_TIME_MAX_VALUE) {
                motion_time = MOTION_TIME_MIN_VALUE;
                ++motion_loop;
            }
            last_exec_time = get_time_ms();
        } else {
            // Motion timeout. Hexapod is not move long time -- need down all limbs
            if (get_time_ms() - last_exec_time > MOTION_LIMBS_DOWN_TIMEOUT) {
                g_hexapod_state = HEXAPOD_STATE_MOTION_DEINIT;
            }
        }
    } 
    else if (g_hexapod_state == HEXAPOD_STATE_MOTION_DEINIT) {
        if (g_ext_motion.cfg.distance == 0) {
            // Move all limbs to down state
            bool is_completed = true;
            for (int32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) { 
                if (!mm_move_value(&g_limbs[i].pos.y, 0.0f, CHANGE_SURFACE_POS_MAX_STEP)) {
                    is_completed = false;
                }
            }
            if (is_completed) {
                motion_time = MOTION_TIME_MIN_VALUE;
                motion_loop = 0;
                g_hexapod_state = HEXAPOD_STATE_DOWN; // Core select corrent state automatically after this function call
            }
        } else {
            g_hexapod_state = HEXAPOD_STATE_MOTION_INIT;
        }
    }
}

/// ***************************************************************************
/// @brief  Load configuration
/// @return true - load and validate success, false - fail
/// ***************************************************************************
static void load_config(void) {
    for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        g_limbs[i].coxa.length  = 53;
        g_limbs[i].coxa.prot_min_angle = -45;
        g_limbs[i].coxa.prot_max_angle = 45;
        g_limbs[i].femur.length = 76;
        g_limbs[i].femur.prot_min_angle = -90;
        g_limbs[i].femur.prot_max_angle = 65;
        g_limbs[i].tibia.length = 137;
        g_limbs[i].tibia.prot_min_angle = -85;
        g_limbs[i].tibia.prot_max_angle = 65;
    }
    
    g_limbs[0].coxa.zero_rotate = 135;
    g_limbs[0].femur.zero_rotate = 35;
    g_limbs[0].tibia.zero_rotate = 135;
    
    g_limbs[1].coxa.zero_rotate = 180;
    g_limbs[1].femur.zero_rotate = 35;
    g_limbs[1].tibia.zero_rotate = 135;
    
    g_limbs[2].coxa.zero_rotate = 225;
    g_limbs[2].femur.zero_rotate = 35;
    g_limbs[2].tibia.zero_rotate = 135;
    
    g_limbs[3].coxa.zero_rotate = 45;
    g_limbs[3].femur.zero_rotate = 35;
    g_limbs[3].tibia.zero_rotate = 135;
    
    g_limbs[4].coxa.zero_rotate = 0;
    g_limbs[4].femur.zero_rotate = 35;
    g_limbs[4].tibia.zero_rotate = 135;
    
    g_limbs[5].coxa.zero_rotate = 315;
    g_limbs[5].femur.zero_rotate = 35;
    g_limbs[5].tibia.zero_rotate = 135;
}





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
