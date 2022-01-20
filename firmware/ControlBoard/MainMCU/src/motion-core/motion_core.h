//  ***************************************************************************
/// @file    motion_core.h
/// @author  NeoProg
/// @brief   Hexapod motion core
//  ***************************************************************************
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

/// ***************************************************************************
/// @brief  Motion core initialization
/// ***************************************************************************
extern void motion_core_init(void);

/// ***************************************************************************
/// @brief  Start/stop motion
/// @param  motion: motion description. @ref motion_t
/// ***************************************************************************
extern void motion_core_move(const motion_t* motion);
extern void motion_core_stop(void);

/// ***************************************************************************
/// @brief  Motion core process
/// @note   Call each PWM period from main loop
/// ***************************************************************************
extern void motion_core_process(void);


#endif /* _MOTION_CORE_H_ */
