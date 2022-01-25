//  ***************************************************************************
/// @file    motion_core.c
/// @author  NeoProg
//  ***************************************************************************
#include "project_base.h"
#include "motion_math.h"
#include <math.h>
#include <float.h>
#define M_PI                                (3.14159265f)
#define RAD_TO_DEG(rad)                     ((rad) * 180.0f / M_PI)
#define DEG_TO_RAD(deg)                     ((deg) * M_PI / 180.0f)
#define IS_EVEN(v)                          ((v & 0x01) == 0)
#define IS_ODD(v)                           ((v & 0x01) != 0)




bool mm_move_value(float* src, float dst, float max_step) {
	float diff = dst - *src;
	float diff_abs = fabs(diff);
    
    // Move completed?
    if (diff_abs < FLT_EPSILON) {
        return false;
    }

    // Constrain step for add remainder
	if (diff_abs < max_step) {
		max_step = diff_abs;
	}

    // Add step
    *src += max_step * (diff / diff_abs);
    return true;
}

bool mm_move_vector(v3d_t* src, const v3d_t* dst, float max_step) {
	float x_diff = dst->x - src->x;
	float y_diff = dst->y - src->y;
	float z_diff = dst->z - src->z;

    // Search max diff
	float max_diff_abs = fabs(x_diff);
	if (max_diff_abs < fabs(y_diff)) {
		max_diff_abs = fabs(y_diff);
	}
	if (max_diff_abs < fabs(z_diff)) {
		max_diff_abs = fabs(z_diff);
	}
    
    // Move completed?
    if (max_diff_abs < FLT_EPSILON) {
        return false;
    }

    // Constrain step for add remainder
	if (max_diff_abs < max_step) {
		max_step = max_diff_abs;
	}

    // Add step
    src->x += max_step * (x_diff / max_diff_abs);
    src->y += max_step * (y_diff / max_diff_abs);
    src->z += max_step * (z_diff / max_diff_abs);
    return true;
}

bool mm_move_surface(p3d_t* src_p, const p3d_t* dst_p, r3d_t* src_r, const r3d_t* dst_r, float max_step) {
	float diff[6] = { 0 };
	diff[0] = dst_p->x - src_p->x;
	diff[1] = dst_p->y - src_p->y;
	diff[2] = dst_p->z - src_p->z;
	diff[3] = dst_r->x - src_r->x;
	diff[4] = dst_r->y - src_r->y;
	diff[5] = dst_r->z - src_r->z;

	// Search max diff
	float max_diff_abs = fabs(diff[0]);
	for (int i = 1; i < sizeof(diff) / sizeof(diff[0]); ++i) {
		if (max_diff_abs < fabs(diff[i])) {
			max_diff_abs = fabs(diff[i]);
		}
	}

	// Move completed?
	if (max_diff_abs < FLT_EPSILON) {
		return false;
	}

	// Constrain step for add remainder
	if (max_diff_abs < max_step) {
		max_step = max_diff_abs;
	}

	// Add step
	src_p->x += max_step * (diff[0] / max_diff_abs);
	src_p->y += max_step * (diff[1] / max_diff_abs);
	src_p->z += max_step * (diff[2] / max_diff_abs);
	src_r->x += max_step * (diff[3] / max_diff_abs);
	src_r->y += max_step * (diff[4] / max_diff_abs);
	src_r->z += max_step * (diff[5] / max_diff_abs);
    
    // Constrain surface rotation angles value
    if (fabs(src_r->x) > 360.0f) {
        src_r->x += -360.0f;
    }
    if (fabs(src_r->y) > 360.0f) {
        src_r->y += -360.0f;
    } 
    if (fabs(src_r->z) > 360.0f) {
        src_r->z += -360.0f;
    }
	return true;
}

bool mm_surface_calculate_offsets(limb_t* limbs, const p3d_t* surface_point, const r3d_t* surface_rotate) {
    v3d_t n = {0, 1, 0};
    float x = 0;
	float y = 0;
	float z = 0;

	// Rotate normal by axis X
	float surface_x_rotate_rad = DEG_TO_RAD(surface_rotate->x);
	y = n.y * cosf(surface_x_rotate_rad) + n.z * sinf(surface_x_rotate_rad);
	z = n.y * sinf(surface_x_rotate_rad) - n.z * cosf(surface_x_rotate_rad);
	n.y = y;
	n.z = z;

	// Rotate normal by axis Z
	float surface_z_rotate_rad = DEG_TO_RAD(surface_rotate->z);
	x = n.x * cosf(surface_z_rotate_rad) - n.y * sinf(surface_z_rotate_rad);
	y = n.x * sinf(surface_z_rotate_rad) + n.y * cosf(surface_z_rotate_rad);
	n.x = x;
	n.y = y;

	// Rotate normal by axis Y
	float surface_y_rotate_rad = DEG_TO_RAD(surface_rotate->y);
	x =  n.x * cosf(surface_y_rotate_rad) + n.z * sinf(surface_y_rotate_rad);
	z = -n.x * sinf(surface_y_rotate_rad) + n.z * cosf(surface_y_rotate_rad);
	n.x = x;
	n.z = z;

    // For avoid divide by zero
    if (n.y == 0) {
        return false;
    }

    // Calculate Y using surface equation. X and Z always is zero
    // Nx(x - x0) + Ny(y - y0) + Nz(z - 0z) = 0
    // y = (-Nx(x - x0) - Nz(z - z0)) / Ny + y0
    for (int32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        limbs[i].surface_offsets.x = surface_point->x;
        limbs[i].surface_offsets.z = surface_point->z;
        limbs[i].surface_offsets.y = -(n.x * (limbs[i].pos.x - surface_point->x) + n.z * (limbs[i].pos.z - surface_point->z)) / n.y + surface_point->y;
    }
    return true;
}

bool mm_kinematic_calculate_angles(limb_t* limbs) {
    for (int32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        float coxa_zero_rotate_deg  = limbs[i].coxa.zero_rotate;
        float femur_zero_rotate_deg = limbs[i].femur.zero_rotate;
        float tibia_zero_rotate_deg = limbs[i].tibia.zero_rotate;
        float coxa_length  = limbs[i].coxa.length;
        float femur_length = limbs[i].femur.length;
        float tibia_length = limbs[i].tibia.length;

        float x = limbs[i].pos.x + limbs[i].surface_offsets.x;
        float y = limbs[i].pos.y + limbs[i].surface_offsets.y;
        float z = limbs[i].pos.z + limbs[i].surface_offsets.z;

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

bool mm_process_advanced_traj(limb_t* limbs, const v3d_t* base_pos, float time, int32_t loop, float curvature, float distance, float step_height) {
    // Scale motion time
    time /= 1000.0f;

    // Check curvature value
    if      ((int32_t)curvature == 0)    curvature = +0.001f;
    else if ((int32_t)curvature > 1000)  curvature = +1000.0f;
    else if ((int32_t)curvature < -1000) curvature = -1000.0f;
    
    // Calculation radius of curvature
    float curvature_radius = expf((1000.0f - fabs(curvature)) / 115.0f) * (curvature / fabs(curvature));

    // Common calculations
    float traj_radius[SUPPORT_LIMBS_COUNT];
    float start_angle_rad[SUPPORT_LIMBS_COUNT];
    float max_traj_radius = 0;
    for (int32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
        // Calculation trajectory radius
        float x0 = base_pos[i].x;
        float z0 = base_pos[i].z;
        traj_radius[i] = sqrtf((curvature_radius - x0) * (curvature_radius - x0) + z0 * z0);

        // Search max trajectory radius
        if (traj_radius[i] > max_traj_radius) {
            max_traj_radius = traj_radius[i];
        }

        // Calculation limb start angle
        start_angle_rad[i] = atan2f(z0, -(curvature_radius - x0));
    }
    if (max_traj_radius == 0) {
        return false; // Avoid division by zero
    }

    // Calculation max angle of arc
    int32_t curvature_radius_sign = (curvature_radius >= 0) ? 1 : -1;
    float max_arc_angle = curvature_radius_sign * distance / max_traj_radius;

    // Calculation points by time
    for (int32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {

        // Inversion motion time if needed
        float relative_motion_time = time;
        if ((loop & 0x01) == 0) {
            if ((i & 0x01) == 0) { // Even?
                relative_motion_time = 1.0f - relative_motion_time;
            }
        } else {
            if ((i & 0x01) != 0) { // Odd?
                relative_motion_time = 1.0f - relative_motion_time;
            }
        }
        
        
        // Calculation arc angle for current time
        float arc_angle_rad = (relative_motion_time - 0.5f) * max_arc_angle + start_angle_rad[i];

        // Calculation XZ points by time
        limbs[i].pos.x = curvature_radius + traj_radius[i] * cosf(arc_angle_rad);
        limbs[i].pos.z =                    traj_radius[i] * sinf(arc_angle_rad);
        
        // Calculation Y points by time
        if ((loop & 0x01) == 0) {
            if ((i & 0x01) == 0) { // Odd?
                limbs[i].pos.y = step_height * sinf(relative_motion_time * M_PI);
            }
        } else {
            if ((i & 0x01) != 0) { // Even?
                limbs[i].pos.y = step_height * sinf(relative_motion_time * M_PI);
            }
        }
    }
    return true;
}
