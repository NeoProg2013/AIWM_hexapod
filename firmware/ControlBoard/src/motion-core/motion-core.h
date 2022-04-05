/// ***************************************************************************
/// @file    motion-core.h
/// @author  NeoProg
/// @brief   Hexapod motion core
/// ***************************************************************************
#ifndef _MOTION_CORE_H_
#define _MOTION_CORE_H_
#include "math-structs.h"

#define MOTION_CTRL_NO                  (0x0000u)
#define MOTION_CTRL_EN_STAB             (0x0001u)


typedef struct {
    uint16_t speed;
    int16_t  curvature;
    int16_t  distance;
    uint16_t step_height;
} motion_cfg_t;

typedef struct {
    // External motion configuration 
    motion_cfg_t cfg;
    uint16_t ctrl;
    // External surface parameters (offset, rotate, some scripts, etc)
    p3d_t surface_point;
    r3d_t surface_rotate;
} ext_motion_t;


extern void motion_core_init(void);
extern void motion_core_move(const ext_motion_t* ext_motion);
extern ext_motion_t motion_core_get_motion(void);
extern void motion_core_process(void);


#endif /* _MOTION_CORE_H_ */
