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
#include <stdlib.h>
#include <math.h>
#define M_PI                                (3.14159265f)
#define RAD_TO_DEG(rad)                     ((rad) * 180.0f / M_PI)
#define DEG_TO_RAD(deg)                     ((deg) * M_PI / 180.0f)

#define SMOOTH_DEFAULT_TOTAL_POINT_COUNT    (30)
#define OVERRIDE_DISABLE_VALUE              (0x7F)


// Servo driver states
typedef enum {
    STATE_NOINIT,
    STATE_WAIT,
    STATE_CALC
} driver_state_t;

typedef enum {
    LINK_COXA,
    LINK_FEMUR,
    LINK_TIBIA
} link_id_t;

typedef struct {
    
    // Current link state
    float angle;
    
    // Link configuration
    uint16_t length;
    int16_t  zero_rotate;
    int16_t  min_angle;
    int16_t  max_angle;
    
} link_info_t;

typedef struct {

    path_type_t path_type;
    point_3d_t  start_point;
    point_3d_t  dest_point;

} path_3d_t;

typedef struct {
    
    point_3d_t position;
    path_3d_t  movement_path;
    
    link_info_t links[3];
    
} limb_info_t;


static int16_t link_angles_override[SUPPORT_LIMB_COUNT * 3] = {0}; 

static driver_state_t driver_state = STATE_NOINIT;
static limb_info_t    limbs[SUPPORT_LIMB_COUNT] = {0};
static bool           is_limbs_move_started = false;
static uint32_t       smooth_total_point_count = SMOOTH_DEFAULT_TOTAL_POINT_COUNT;


static bool read_configuration(void);
static void path_calculate_point(const path_3d_t* info, point_3d_t* point, uint32_t smooth_current_point);
static bool kinematic_calculate_angles(limb_info_t* info);


//  ***************************************************************************
/// @brief  Limbs driver initialize
/// @param  none
/// @return none
//  ***************************************************************************
void limbs_driver_init(void) {
    
    if (read_configuration() == false) {
        sysmon_set_error(SYSMON_CONFIG_ERROR);
        sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
        return;
    }
    
    // Initialization override variables
    for (uint32_t i = 0; i < SUPPORT_LIMB_COUNT * 3; ++i) {
        link_angles_override[i] = OVERRIDE_DISABLE_VALUE;
    }
    
    // Calculate start link angles
    for (uint32_t i = 0; i < SUPPORT_LIMB_COUNT; ++i) {
        
        if (kinematic_calculate_angles(&limbs[i]) == false) {
            sysmon_set_error(SYSMON_CONFIG_ERROR);
            sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
            return;
        }
    }
    
    // Set start servo angles
    for (uint32_t i = 0; i < SUPPORT_LIMB_COUNT; ++i) {
        servo_driver_move(i * 3 + 0, limbs[i].links[LINK_COXA].angle);
        servo_driver_move(i * 3 + 1, limbs[i].links[LINK_FEMUR].angle);
        servo_driver_move(i * 3 + 2, limbs[i].links[LINK_TIBIA].angle);
    }
    
    // Initialization driver state
    is_limbs_move_started = false;
    driver_state = STATE_WAIT;
}

//  ***************************************************************************
/// @brief  Start smooth algorithm configuration
/// @param  point_count: smooth point count
//  ***************************************************************************
void limbs_driver_set_smooth_config(uint32_t point_count) {
    
    smooth_total_point_count = point_count;

    if (smooth_total_point_count == 0) {
        sysmon_set_error(SYSMON_FATAL_ERROR);
        sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
    }
}

//  ***************************************************************************
/// @brief  Start limb move
/// @param  point_list: destination point list
/// @param  path_type_list: path type list
//  ***************************************************************************
void limbs_driver_start_move(const point_3d_t* point_list, const path_type_t* path_type_list) {
    
    if (point_list == NULL) {
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
    }
}

//  ***************************************************************************
/// @brief  Check all limbs movement complete
/// @param  none
/// @return true - move complete, false - movement in progress
//  ***************************************************************************
bool limbs_driver_is_move_complete(void) {
    
    return is_limbs_move_started == false;
}

//  ***************************************************************************
/// @brief  Limbs driver process
/// @note   Call from main loop
//  ***************************************************************************
void limbs_driver_process(void) {
    
    if (sysmon_is_module_disable(SYSMON_MODULE_LIMBS_DRIVER) == true) return;  // Module disabled
    

    static uint32_t smooth_current_point = 0;
    static uint32_t prev_synchro_value = 0xFFFFFFFF;
    
    switch (driver_state) {
        
        case STATE_WAIT:
            if (synchro != prev_synchro_value) {
                
                if (synchro - prev_synchro_value > 1 && prev_synchro_value != 0xFFFFFFFF) {
                    sysmon_set_error(SYSMON_SYNC_ERROR);
                    sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
                    return;
                }
                prev_synchro_value = synchro;
                driver_state = STATE_CALC;
            }
            break;
        
        case STATE_CALC:
            //
            // Calculate new servo angles
            //
            if (is_limbs_move_started == true) {
                
                for (uint32_t i = 0; i < SUPPORT_LIMB_COUNT; ++i) {
                
                    // Calculate next point
                    path_calculate_point(&limbs[i].movement_path, &limbs[i].position, smooth_current_point);
                    
                    // Calculate angles for point
                    if (kinematic_calculate_angles(&limbs[i]) == false) {
                        sysmon_set_error(SYSMON_MATH_ERROR);
                        sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
                        return;
                    }
                }
                ++smooth_current_point;
                
                if (smooth_current_point > smooth_total_point_count) {
                    is_limbs_move_started = false;
                    smooth_current_point = 0;
                }
            }            
               
            //
            // Load new angles to servo driver
            //
            for (uint32_t i = 0; i < SUPPORT_LIMB_COUNT; ++i) {
                 
                // Override process
                if (link_angles_override[i * 3 + 0] != OVERRIDE_DISABLE_VALUE) {
                    limbs[i].links[LINK_COXA].angle = link_angles_override[i * 3 + 0];
                }
                if (link_angles_override[i * 3 + 1] != OVERRIDE_DISABLE_VALUE) {
                    limbs[i].links[LINK_FEMUR].angle = link_angles_override[i * 3 + 1];
                }
                if (link_angles_override[i * 3 + 2] != OVERRIDE_DISABLE_VALUE) {
                    limbs[i].links[LINK_TIBIA].angle = link_angles_override[i * 3 + 2];
                }
                                
                // Move servos to destination angles
                servo_driver_move(i * 3 + 0, limbs[i].links[LINK_COXA].angle);
                servo_driver_move(i * 3 + 1, limbs[i].links[LINK_FEMUR].angle);
                servo_driver_move(i * 3 + 2, limbs[i].links[LINK_TIBIA].angle);
            }
            driver_state = STATE_WAIT;
            break;
        
        case STATE_NOINIT:
        default:
            sysmon_set_error(SYSMON_FATAL_ERROR);
            sysmon_disable_module(SYSMON_MODULE_LIMBS_DRIVER);
            break;
    }
}





//  ***************************************************************************
/// @brief  Read configuration
/// @param  none
/// @return true - read success, false - fail
//  ***************************************************************************
static bool read_configuration(void) {
    
    /*for (uint32_t i = 0; i < SUPPORT_LIMB_COUNT; ++i) {
        
        uint32_t base_address = MM_LIMB_CONFIG_BLOCK_BASE_EE_ADDRESS + i * MM_LIMB_CONFIG_BLOCK_SIZE;
        
        // Read coxa, femur and tibia lengths
        config_read_16(base_address + MM_LIMB_COXA_LENGTH_OFFSET,  &limbs[i].links[LINK_COXA].length);
        config_read_16(base_address + MM_LIMB_FEMUR_LENGTH_OFFSET, &limbs[i].links[LINK_FEMUR].length);
        config_read_16(base_address + MM_LIMB_TIBIA_LENGTH_OFFSET, &limbs[i].links[LINK_TIBIA].length);
        if (limbs[i].links[LINK_COXA].length == 0xFFFF || limbs[i].links[LINK_FEMUR].length == 0xFFFF || limbs[i].links[LINK_TIBIA].length == 0xFFFF) {
            return false;
        }
        
        // Read coxa, femur and tibia zero rotate
        config_read_16(base_address + MM_LIMB_COXA_ZERO_ROTATE_OFFSET,  (uint16_t*)&limbs[i].links[LINK_COXA].zero_rotate);
        config_read_16(base_address + MM_LIMB_FEMUR_ZERO_ROTATE_OFFSET, (uint16_t*)&limbs[i].links[LINK_FEMUR].zero_rotate);
        config_read_16(base_address + MM_LIMB_TIBIA_ZERO_ROTATE_OFFSET, (uint16_t*)&limbs[i].links[LINK_TIBIA].zero_rotate);
        
        if (limbs[i].links[LINK_COXA].zero_rotate < -360  || limbs[i].links[LINK_COXA].zero_rotate > 360  || 
            limbs[i].links[LINK_FEMUR].zero_rotate < -360 || limbs[i].links[LINK_FEMUR].zero_rotate > 360 ||
            limbs[i].links[LINK_TIBIA].zero_rotate < -360 || limbs[i].links[LINK_TIBIA].zero_rotate > 360) {
            return false;
        }

        // Read coxa, femur and tibia start position
        uint16_t buffer[3];
        config_read_16(base_address + MM_LIMB_START_POSITION_X_OFFSET, &buffer[0]);
        config_read_16(base_address + MM_LIMB_START_POSITION_Y_OFFSET, &buffer[1]);
        config_read_16(base_address + MM_LIMB_START_POSITION_Z_OFFSET, &buffer[2]);
        if (buffer[0] == 0xFFFF || buffer[1] == 0xFFFF || buffer[2] == 0xFFFF) {
            return false;
        }
        
        // Convert int16_t to float
        limbs[i].position.x = (int16_t)buffer[0];
        limbs[i].position.y = (int16_t)buffer[1];
        limbs[i].position.z = (int16_t)buffer[2];
    }*/
    
    // COXA = 53mm
    // FEMUR = 76mm
    // TIBIA = 137mm
    
    // X^FEMUR = 125
    // FEMUR^TIBIA = 45
    
    limbs[0].position.x = limbs[3].position.x = 140;
    limbs[0].position.y = limbs[3].position.y = -25;
    limbs[0].position.z = limbs[3].position.z = 65;
    limbs[0].links[LINK_COXA].length       = limbs[3].links[LINK_COXA].length       = 53;
    limbs[0].links[LINK_COXA].zero_rotate  = limbs[3].links[LINK_COXA].zero_rotate  = 45;
    limbs[0].links[LINK_FEMUR].length      = limbs[3].links[LINK_FEMUR].length      = 76;
    limbs[0].links[LINK_FEMUR].zero_rotate = limbs[3].links[LINK_FEMUR].zero_rotate = 125;
    limbs[0].links[LINK_TIBIA].length      = limbs[3].links[LINK_TIBIA].length      = 137;
    limbs[0].links[LINK_TIBIA].zero_rotate = limbs[3].links[LINK_TIBIA].zero_rotate = 45;
    
    limbs[1].position.x = limbs[4].position.x = 150;
    limbs[1].position.y = limbs[4].position.y = -25;
    limbs[1].position.z = limbs[4].position.z = 0;
    limbs[1].links[LINK_COXA].length       = limbs[4].links[LINK_COXA].length       = 53;
    limbs[1].links[LINK_COXA].zero_rotate  = limbs[4].links[LINK_COXA].zero_rotate  = 0;
    limbs[1].links[LINK_FEMUR].length      = limbs[4].links[LINK_FEMUR].length      = 76;
    limbs[1].links[LINK_FEMUR].zero_rotate = limbs[4].links[LINK_FEMUR].zero_rotate = 125;
    limbs[1].links[LINK_TIBIA].length      = limbs[4].links[LINK_TIBIA].length      = 137;
    limbs[1].links[LINK_TIBIA].zero_rotate = limbs[4].links[LINK_TIBIA].zero_rotate = 45;
    
    limbs[2].position.x = limbs[5].position.x = 140;
    limbs[2].position.y = limbs[5].position.y = -25;
    limbs[2].position.z = limbs[5].position.z = -65;
    limbs[2].links[LINK_COXA].length       = limbs[5].links[LINK_COXA].length       = 53;
    limbs[2].links[LINK_COXA].zero_rotate  = limbs[5].links[LINK_COXA].zero_rotate  = -45;
    limbs[2].links[LINK_FEMUR].length      = limbs[5].links[LINK_FEMUR].length      = 76;
    limbs[2].links[LINK_FEMUR].zero_rotate = limbs[5].links[LINK_FEMUR].zero_rotate = 125;
    limbs[2].links[LINK_TIBIA].length      = limbs[5].links[LINK_TIBIA].length      = 137;
    limbs[2].links[LINK_TIBIA].zero_rotate = limbs[5].links[LINK_TIBIA].zero_rotate = 45;

    return true;
}

//  ***************************************************************************
/// @brief  Calculate path point
/// @param  info: path info @ref path_3d_t
/// @param  point: calculated point
/// @retval point
//  ***************************************************************************
static void path_calculate_point(const path_3d_t* info, point_3d_t* point, uint32_t smooth_current_point) {
    
    float t_max = RAD_TO_DEG(M_PI); // [0; Pi]
    float t = smooth_current_point * (t_max / smooth_total_point_count); // iter_index * dt

    float x0 = info->start_point.x;
    float y0 = info->start_point.y;
    float z0 = info->start_point.z;
    float x1 = info->dest_point.x;
    float y1 = info->dest_point.y;
    float z1 = info->dest_point.z;


    if (info->path_type == PATH_LINEAR) {
        point->x = t * (x1 - x0) / t_max + x0;
        point->y = t * (y1 - y0) / t_max + y0;
        point->z = t * (z1 - z0) / t_max + z0;
    }
    else
    if (info->path_type == PATH_XZ_ARC_Y_LINEAR) {

        float R = sqrt(x0 * x0 + z0 * z0);
        float atan0 = RAD_TO_DEG(atan2f(x0, z0));
        float atan1 = RAD_TO_DEG(atan2f(x1, z1));

        float t_mapped_rad = DEG_TO_RAD(t * (atan1 - atan0) / t_max + atan0);
        point->x = R * sinf(t_mapped_rad);                   // Arc Y
        point->y = t * (y1 - y0) / t_max + y0;
        point->z = R * cosf(t_mapped_rad);                   // Arc X
    }
    else
    if (info->path_type == PATH_XZ_ARC_Y_SINUS) {
        
        float R = sqrt(x0 * x0 + z0 * z0);
        float atan0 = RAD_TO_DEG(atan2f(x0, z0));
        float atan1 = RAD_TO_DEG(atan2f(x1, z1));

        float t_mapped_rad = DEG_TO_RAD(t * (atan1 - atan0) / t_max + atan0);
        point->x = R * sinf(t_mapped_rad);                   // Arc Y
        point->y = (y1 - y0) * sinf(DEG_TO_RAD(t)) + y0;
        point->z = R * cosf(t_mapped_rad);                   // Arc X
    }
    else
    if (info->path_type == PATH_XZ_ELLIPTICAL_Y_SINUS) {

        float a = (z1 - z0) / 2.0f;
        float b = (x1 - x0);
        float c = (y1 - y0);

        point->x = b * sinf(DEG_TO_RAD(t_max - t)) + x0;     // Ellipse Y
        point->y = c * sinf(DEG_TO_RAD(t)) + y0;
        point->z = a * cosf(DEG_TO_RAD(t_max - t)) + z0 + a; // Ellipse X
    }
    else
    if (info->path_type == PATH_YZ_ELLIPTICAL_X_SINUS) {

		float a = (x1 - x0) / 2.0f;
		float b = (z1 - z0);
		float c = (y1 - y0);

		point->x = a * cosf(DEG_TO_RAD(t_max - t)) + x0 + a; // Ellipse X
		point->y = c * sinf(DEG_TO_RAD(t)) + y0;
		point->z = b * sinf(DEG_TO_RAD(t_max - t)) + z0;     // Ellipse Y
	}
}

//  ***************************************************************************
/// @brief  Calculate angles
/// @param  info: limb info @ref limb_info_t
/// @return true - calculation success, false - no
//  ***************************************************************************
static bool kinematic_calculate_angles(limb_info_t* info) {
    
    float coxa_zero_rotate_deg  = info->links[LINK_COXA].zero_rotate;
    float femur_zero_rotate_deg = info->links[LINK_FEMUR].zero_rotate;
    float tibia_zero_rotate_deg = info->links[LINK_TIBIA].zero_rotate;
    float coxa_length  = info->links[LINK_COXA].length;
    float femur_length = info->links[LINK_FEMUR].length;
    float tibia_length = info->links[LINK_TIBIA].length;
    
    float x = info->position.x;
    float y = info->position.y;
    float z = info->position.z;
    
    
    // Move to (X*, Y*, Z*) coordinate system - rotate
    float coxa_zero_rotate_rad = DEG_TO_RAD(coxa_zero_rotate_deg);
    float x1 = x * cosf(coxa_zero_rotate_rad) + z * sinf(coxa_zero_rotate_rad);
    float y1 = y;
    float z1 = -x * sinf(coxa_zero_rotate_rad) + z * cosf(coxa_zero_rotate_rad);


    //
    // Calculate COXA angle
    //
    float coxa_angle_rad = atan2f(z1, x1);
    info->links[LINK_COXA].angle = RAD_TO_DEG(coxa_angle_rad);


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
    info->links[LINK_FEMUR].angle = femur_zero_rotate_deg - RAD_TO_DEG(alpha) - RAD_TO_DEG(fi);
    info->links[LINK_TIBIA].angle = RAD_TO_DEG(gamma) - tibia_zero_rotate_deg;
    return true;
}
