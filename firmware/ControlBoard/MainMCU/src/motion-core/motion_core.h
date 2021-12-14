//  ***************************************************************************
/// @file    motion_core.h
/// @author  NeoProg
/// @brief   Hexapod motion core
//  ***************************************************************************
#ifndef _MOTION_CORE_H_
#define _MOTION_CORE_H_
#include "math-structs.h"


typedef struct {
    p3d_t   surface_point;
    r3d_t   surface_rotate;
    int16_t speed;
    int16_t curvature;
    int16_t distance;
} motion_t;


extern void motion_core_init(void);
extern void motion_core_get_current_motion(motion_t* motion);
extern void motion_core_start_motion(const motion_t* motion);
extern void motion_core_stop_motion(void);
extern void motion_core_update_motion(const motion_t* motion);
extern void motion_core_process(void);


#endif /* _MOTION_CORE_H_ */
