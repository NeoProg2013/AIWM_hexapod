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
/// @param  x_rotate, z_rotate: hexapod plane rotate relative ground plane
/// @return true - calculation success, false - no
/// ***************************************************************************
void surface_compensation(limb_t* limbs, int32_t limbs_cnt, float surface_x_rotate, float surface_z_rotate) {
    const float max_rotate_angle = 6.4f;
    if (surface_z_rotate < -max_rotate_angle) {
        surface_z_rotate = -max_rotate_angle;
    } else if (surface_z_rotate > max_rotate_angle) {
        surface_z_rotate = max_rotate_angle;
    }
    if (surface_x_rotate < -max_rotate_angle) {
        surface_x_rotate = -max_rotate_angle;
    } else if (surface_x_rotate > max_rotate_angle) {
        surface_x_rotate = max_rotate_angle;
    }

    // Surface normal
    v3d_t n;
    n.x = 0;
    n.y = 1;
    n.z = 0;

    // Move surface to max position
    v3d_t a = {0, 0, 0};
    if (surface_z_rotate < 0) a.x = +(135.0f + 80.f);
    else                      a.x = -(135.0f + 80.f);
    if (surface_z_rotate < 0) a.z = +(70.0f + 104.0f);
    else                      a.z = -(70.0f + 104.0f);

    // Rotate normal by axis X
    float surface_x_rotate_rad = DEG_TO_RAD(surface_x_rotate);
    n.y = n.y * cosf(surface_x_rotate_rad) + n.z * sinf(surface_x_rotate_rad);
    n.z = n.y * sinf(surface_x_rotate_rad) - n.z * cosf(surface_x_rotate_rad);

    // Rotate normal by axis Z
    float surface_z_rotate_rad = DEG_TO_RAD(surface_z_rotate);
    n.x = n.x * cosf(surface_z_rotate_rad) + n.y * sinf(surface_z_rotate_rad);
    n.y = n.x * sinf(surface_z_rotate_rad) - n.y * cosf(surface_z_rotate_rad);

    // Calculate Y offsets
    for (int32_t i = 0; i < limbs_cnt; ++i) {
        float z_sign = 1.0f;
        if (limbs[i].pos.z != 0) {
            z_sign = limbs[i].pos.z / fabs(limbs[i].pos.z);
        }
        float z = limbs[i].pos.z + z_sign * 104.0f; // 104 - coxa position from center by axix Z

        float x_sign = 1.0f;
        if (limbs[i].pos.x != 0) {
            x_sign = limbs[i].pos.x / fabs(limbs[i].pos.x);
        }
        float x = limbs[i].pos.x + x_sign * 104.0f;

        limbs[i].surface_comp = (-1) * (-n.z * (z - a.z) - n.x * (x - a.x)) / n.y;
    }
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
        float y = limbs[i].pos.y + limbs[i].surface_comp;
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