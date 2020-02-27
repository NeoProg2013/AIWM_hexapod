//  ***************************************************************************
/// @file    limbs_driver.h
/// @author  NeoProg
/// @brief   Hexapod limbs driver
//  ***************************************************************************
#ifndef LIMB_H_
#define LIMB_H_

#include <stdint.h>
#include <stdbool.h>

#define SUPPORT_LIMB_COUNT                (6)


typedef struct {
    float x;
    float y;
    float z;
} point_3d_t;

typedef enum {
    PATH_LINEAR,
    PATH_XZ_ARC_Y_LINEAR,
    PATH_XZ_ARC_Y_SINUS,
    PATH_XZ_ELLIPTICAL_Y_SINUS,
    PATH_YZ_ELLIPTICAL_X_SINUS,
} path_type_t;


extern void limbs_driver_init(const point_3d_t* point_list);
extern void limbs_driver_set_smooth_config(uint32_t point_count);
extern void limbs_driver_start_move(const point_3d_t* point_list, const path_type_t* path_type_list);
extern void limbs_driver_process(void);
extern bool limbs_driver_is_move_complete(void);


#endif /* LIMB_H_ */