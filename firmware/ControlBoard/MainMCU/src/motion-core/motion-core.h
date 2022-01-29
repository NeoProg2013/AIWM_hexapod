/// ***************************************************************************
/// @file    motion-core.h
/// @author  NeoProg
/// @brief   Hexapod motion core
/// ***************************************************************************
#ifndef _MOTION_CORE_H_
#define _MOTION_CORE_H_
#include "math-structs.h"

typedef struct {
    int16_t speed;
    int16_t curvature;
    int16_t distance;
    int16_t step_height;
} motion_cfg_t;

typedef struct {
    p3d_t surface_point;
    r3d_t surface_rotate;
    motion_cfg_t cfg;
} motion_t;

typedef enum {
    MOTION_SCRIPT_NONE = -1,
    MOTION_SCRIPT_UP,
    MOTION_SCRIPT_DOWN,
    MOTION_SCRIPT_X_ROTATE,
    MOTION_SCRIPT_Z_ROTATE,
    MOTION_SCRIPT_XY_ROTATE,
    MOTION_SCRIPT_UP_DOWN,
    MOTION_SCRIPT_Z_PUSH_PULL,
    MOTION_SCRIPT_X_SWAY,
    MOTION_SCRIPT_SQUARE,

    MOTION_SCRIPTS_COUNT
} motion_script_id_t;


/// ***************************************************************************
/// @brief  Motion core initialization
/// ***************************************************************************
extern void motion_core_init(void);

/// ***************************************************************************
/// @brief  Start/stop main motion
/// @param  motion: motion description. @ref motion_t
/// ***************************************************************************
extern void motion_core_move(const motion_t* motion);
extern void motion_core_stop(void);

/// ***************************************************************************
/// @brief  Start/stop motion script
/// @param  id: motion script id. @ref motion_script_id_t
/// ***************************************************************************
extern void motion_core_select_script(motion_script_id_t id);

/// ***************************************************************************
/// @brief  Get current motion parameters
/// @return Copy of motion parameters
/// ***************************************************************************
extern motion_t motion_core_get_current_motion(void);

/// ***************************************************************************
/// @brief  Motion core process
/// @note   Call each PWM period from main loop
/// ***************************************************************************
extern void motion_core_process(void);


#endif /* _MOTION_CORE_H_ */
