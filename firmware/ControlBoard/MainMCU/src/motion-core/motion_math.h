//  ***************************************************************************
/// @file    motion_core.h
/// @author  NeoProg
/// @brief   Hexapod motion core
//  ***************************************************************************
#ifndef _MOTION_MATH_H_
#define _MOTION_MATH_H_
#include "math-structs.h"

#define SUPPORT_LIMBS_COUNT                 (6)


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
extern bool  mm_surface_calculate_offsets(limb_t* limbs, const p3d_t* surface_point, const r3d_t* surface_rotate);
extern bool  mm_kinematic_calculate_angles(limb_t* limbs);
extern bool  mm_process_advanced_traj(limb_t* limbs, const v3d_t* limbs_base_pos, float time, int32_t loop, float curvature, float distance, float step_height);


#endif // _MOTION_MATH_H_
