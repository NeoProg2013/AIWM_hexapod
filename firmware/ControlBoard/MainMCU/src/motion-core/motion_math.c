//  ***************************************************************************
/// @file    motion_core.c
/// @author  NeoProg
//  ***************************************************************************
#include "project_base.h"
#include "motion_math.h"
#include <math.h>
#define M_PI                                (3.14159265f)
#define RAD_TO_DEG(rad)                     ((rad) * 180.0f / M_PI)
#define DEG_TO_RAD(deg)                     ((deg) * M_PI / 180.0f)



/// ***************************************************************************
/// @brief  Surface compensation
/// @param  limbs, limbs_cnt: hexapod limbs
/// @param  surface_point: surface point
/// @param  rotate: surface rotate
/// @return true - calculation success, false - no
/// ***************************************************************************
bool surface_calculate_offset(limb_t* limbs, int32_t limbs_cnt, const p3d_t* surface_point, const r3d_t* surface_rotate) {
    v3d_t n = {0, 1, 0};

    // Rotate normal by axis X
    float surface_x_rotate_rad = DEG_TO_RAD(surface_rotate->x);
    n.y = n.y * cosf(surface_x_rotate_rad) + n.z * sinf(surface_x_rotate_rad);
    n.z = n.y * sinf(surface_x_rotate_rad) - n.z * cosf(surface_x_rotate_rad);

    // Rotate normal by axis Z
    float surface_z_rotate_rad = DEG_TO_RAD(surface_rotate->z);
    n.x = n.x * cosf(surface_z_rotate_rad) + n.y * sinf(surface_z_rotate_rad);
    n.y = n.x * sinf(surface_z_rotate_rad) - n.y * cosf(surface_z_rotate_rad);

    // For avoid divide by zero
    if (n.y == 0) {
        return false;
    }

    // Calculate Y using surface equation. X and Z always is zero
    // Nx(x - x0) + Ny(y - y0) + Nz(z - 0z) = 0
    // y = (-Nx(x - x0) - Nz(z - z0)) / Ny + y0
    for (int32_t i = 0; i < limbs_cnt; ++i) {
        limbs[i].surface_y_offset = (-n.z * (surface_point->z - limbs[i].pos.z) - n.x * (surface_point->x - limbs[i].pos.x)) / n.y + surface_point->y;
    }
    return true;
}

//  ***************************************************************************
/// @brief  Calculate angles
/// @param  limbs: limb_t structure, @ref limb_t
/// @retval limb_t::link_t::servo_angle
/// @return true - calculation success, false - no
//  ***************************************************************************
bool kinematic_calculate_angles(limb_t* limbs, int32_t limbs_cnt) {
    for (int32_t i = 0; i < limbs_cnt; ++i) {
        float coxa_zero_rotate_deg  = limbs[i].coxa.zero_rotate;
        float femur_zero_rotate_deg = limbs[i].femur.zero_rotate;
        float tibia_zero_rotate_deg = limbs[i].tibia.zero_rotate;
        float coxa_length  = limbs[i].coxa.length;
        float femur_length = limbs[i].femur.length;
        float tibia_length = limbs[i].tibia.length;

        float x = limbs[i].pos.x;
        float y = limbs[i].pos.y + limbs[i].surface_y_offset;
        float z = limbs[i].pos.z;


        // Move to (X*, Y*, Z*) coordinate system - rotate
        float coxa_zero_rotate_rad = DEG_TO_RAD(coxa_zero_rotate_deg);
        float x1 = x * cosf(coxa_zero_rotate_rad) + z * sinf(coxa_zero_rotate_rad);
        float y1 = y;
        float z1 = -x * sinf(coxa_zero_rotate_rad) + z * cosf(coxa_zero_rotate_rad);


        // Calculate COXA angle
        float coxa_angle_rad = atan2f(z1, x1);
        limbs[i].coxa.angle = RAD_TO_DEG(coxa_angle_rad);


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


        // Calculate triangle angles
        float a = tibia_length;
        float b = femur_length;
        float c = d;
        float alpha = acosf( (b * b + c * c - a * a) / (2.0f * b * c) );
        float gamma = acosf( (a * a + b * b - c * c) / (2.0f * a * b) );

        // Calculate FEMUR and TIBIA angle
        limbs[i].femur.angle = femur_zero_rotate_deg - RAD_TO_DEG(alpha) - RAD_TO_DEG(fi);
        limbs[i].tibia.angle = RAD_TO_DEG(gamma) - tibia_zero_rotate_deg;

        // Protection
        if (limbs[i].coxa.angle  < limbs[i].coxa.prot_min_angle)  limbs[i].coxa.angle  = limbs[i].coxa.prot_min_angle;
        if (limbs[i].coxa.angle  > limbs[i].coxa.prot_max_angle)  limbs[i].coxa.angle  = limbs[i].coxa.prot_max_angle;
        if (limbs[i].femur.angle < limbs[i].femur.prot_min_angle) limbs[i].femur.angle = limbs[i].femur.prot_min_angle;
        if (limbs[i].femur.angle > limbs[i].femur.prot_max_angle) limbs[i].femur.angle = limbs[i].femur.prot_max_angle;
        if (limbs[i].tibia.angle < limbs[i].tibia.prot_min_angle) limbs[i].tibia.angle = limbs[i].tibia.prot_min_angle;
        if (limbs[i].tibia.angle > limbs[i].tibia.prot_max_angle) limbs[i].tibia.angle = limbs[i].tibia.prot_max_angle;
    }
    return true;
}