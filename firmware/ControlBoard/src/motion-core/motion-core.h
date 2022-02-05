/// ***************************************************************************
/// @file    motion-core.h
/// @author  NeoProg
/// @brief   Hexapod motion core
/// ***************************************************************************
#ifndef _MOTION_CORE_H_
#define _MOTION_CORE_H_
#include "math-structs.h"

typedef struct {
    uint16_t speed;
    int16_t  curvature;
    int16_t  distance;
    uint16_t step_height;
} motion_cfg_t;

typedef struct {
    // Surface parameters from user
    p3d_t user_surface_point;
    r3d_t user_surface_rotate;
    // Surface parameters for internal usage
    p3d_t surface_point;
    r3d_t surface_rotate;
    // Motion configuration from user
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



extern void motion_core_init(void);
extern void motion_core_move(const motion_t* motion, motion_script_id_t id);
extern motion_t motion_core_get_current_motion(void);
extern void motion_core_process(void);


#endif /* _MOTION_CORE_H_ */
