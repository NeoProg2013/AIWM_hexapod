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
    v3d_t  pos; // Limb position on flat surface with (0; 0; 0) coords and normal vector (0; 1; 0)
    v3d_t  surface_offset;
    link_t coxa;
    link_t femur;
    link_t tibia;
} limb_t;


extern float mm_calc_step(float src, float dst, float max_step);
extern bool  mm_surface_calculate_offsets(limb_t* limbs, int32_t limbs_cnt, const p3d_t* surface_point, const r3d_t* surface_rotate);
extern bool  mm_kinematic_calculate_angles(limb_t* limbs, int32_t limbs_cnt);


#endif // _MOTION_MATH_H_
