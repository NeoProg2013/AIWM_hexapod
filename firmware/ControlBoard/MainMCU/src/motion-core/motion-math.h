//  ***************************************************************************
/// @file    motion-core.h
/// @author  NeoProg
/// @brief   Hexapod motion core
//  ***************************************************************************
#ifndef _MOTION_MATH_H_
#define _MOTION_MATH_H_
#include "math-structs.h"

#define SUPPORT_LIMBS_COUNT                 (6)


typedef struct {
    float    angle;
    uint16_t length;         // [CFG]
    int16_t  zero_rotate;    // [CFG]
    int16_t  prot_min_angle; // [CFG] Protection min angle, [degree]
    int16_t  prot_max_angle; // [CFG] Protection max angle, [degree]
} link_t;

typedef struct {
    v3d_t  pos;              // Limb position on flat surface with (0; 0; 0) coords and normal vector (0; 1; 0)
    v3d_t  surface_offsets;  // Limb position relatively surface height from (0; 0; 0) and rotate
    link_t coxa;
    link_t femur;
    link_t tibia;
} limb_t;



/// ***************************************************************************
/// @brief  Move value on step
/// @param  src: source value
/// @param  dst: destination value
/// @param  max_step: max step for move
/// @return true - move success, false - value already reached destination value
/// ***************************************************************************
extern bool mm_move_value(float* src, float dst, float max_step);

/// ***************************************************************************
/// @brief  Move vector on step
/// @param  src: source vector pos
/// @param  dst: destination vector pos
/// @param  max_step: max step for move
/// @return true - move success, false - vector already reached destination pos
/// ***************************************************************************
extern bool mm_move_vector(v3d_t* src, const v3d_t* dst, float max_step);

/// ***************************************************************************
/// @brief  Move surface on step
/// @param  src_p: source surface point pos
/// @param  dst_p: destination surface point pos
/// @param  src_r: source surface rotation
/// @param  dst_r: destination surface rotation
/// @param  max_step: max step for move
/// @return true - move success, false - furface already reached destination pos
/// ***************************************************************************
extern bool mm_move_surface(p3d_t* src_p, const p3d_t* dst_p, r3d_t* src_r, const r3d_t* dst_r, float max_step);

/// ***************************************************************************
/// @brief  Surface compensation
/// @param  limbs: hexapod limbs
/// @param  surface_point: surface point
/// @param  surface_rotate: surface rotate
/// @return true - calculation success, false - no
/// ***************************************************************************
extern bool mm_surface_calculate_offsets(limb_t* limbs, const p3d_t* surface_point, const r3d_t* surface_rotate);

//  ***************************************************************************
/// @brief  Calculate angles
/// @param  limbs: limb_t structure, @ref limb_t
/// @retval limb_t::link_t::servo_angle
/// @return true - calculation success, false - no
//  ***************************************************************************
extern bool mm_kinematic_calculate_angles(limb_t* limbs);

/// ***************************************************************************
/// @brief  Process advanced trajectory
/// @param  limbs: limb_t structure, @ref limb_t
/// @param  base_pos: base limbs position
/// @param  time: current motion time [0; 1000]
/// @param  loop: current motion loop
/// @param  curvature: trajectory curvature
/// @param  distance: trajectory distance
/// @param  step_height: max step height
/// @retval modify g_limbs::pos
/// @return true - calculation success, false - no
/// ***************************************************************************
extern bool mm_process_advanced_traj(limb_t* limbs, const v3d_t* base_pos, float time, int32_t loop, float curvature, float distance, float step_height);


#endif // _MOTION_MATH_H_
