//  ***************************************************************************
/// @file    motion_core.h
/// @author  NeoProg
/// @brief   Hexapod motion core
//  ***************************************************************************
#ifndef _MOTION_MATH_H_
#define _MOTION_MATH_H_
#include "math-structs.h"


typedef struct {
    float    angle;
    uint16_t length;
    int16_t  zero_rotate;
    int16_t  prot_min_angle; // Protection min angle, [degree]
    int16_t  prot_max_angle; // Protection max angle, [degree]
} link_t;

typedef struct {
    v3d_t  pos;
    float  surface_y_offset;
    link_t coxa;
    link_t femur;
    link_t tibia;
} limb_t;


extern bool surface_calculate_offset(limb_t* limbs, int32_t limbs_cnt, const p3d_t* surface_point, const r3d_t* surface_rotate);
extern bool kinematic_calculate_angles(limb_t* limbs, int32_t limbs_cnt);


#endif // _MOTION_MATH_H_
