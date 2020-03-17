//  ***************************************************************************
/// @file    motion_core.h
/// @author  NeoProg
/// @brief   Hexapod motion core
//  ***************************************************************************
#ifndef _MOTION_CORE_H_
#define _MOTION_CORE_H_

#include <stdint.h>
#include <stdbool.h>

#define SUPPORT_LIMBS_COUNT                 (6)

#define MTIME_SCALE                         (1000)
#define MTIME_MIN_VALUE                     (0 * MTIME_SCALE)
#define MTIME_MAX_VALUE                     (1 * MTIME_SCALE)
#define MTIME_MID_VALUE                     (MTIME_MAX_VALUE >> 1)
#define MTIME_NO_UPDATE                     (MTIME_MAX_VALUE << 1)


typedef enum {
	TRAJECTORY_XYZ_LINEAR,
	TRAJECTORY_XZ_ADV_Y_CONST,
	TRAJECTORY_XZ_ADV_Y_SINUS
} trajectory_t;

typedef enum {
	TIME_DIR_REVERSE = -1,
	TIME_DIR_DIRECT = 1
} time_dir_t;

typedef enum {
    MOTION_FLAG_NO,
    MOTION_FLAG_NOT_INIT_START_POINTS = 0x01,
} motion_flags_t;

typedef struct {
	float x;
	float y;
	float z;
} point_3d_t;

typedef struct {
	point_3d_t dest_positions[SUPPORT_LIMBS_COUNT];     // Destination point for each limb
	trajectory_t trajectories[SUPPORT_LIMBS_COUNT];     // Motion trajectory
	time_dir_t time_directions[SUPPORT_LIMBS_COUNT];    // Motion time direction for each limb
    point_3d_t start_positions[SUPPORT_LIMBS_COUNT];    // Initialize auto when motion started
	int16_t    curvature;                               // Curvature (using for advanced trajectory)
	uint16_t   step_length;                             // Step length (using for advanced trajectory)
	uint16_t   step_height;                             // Step height (using for advanced trajectory)
	uint16_t   time_start;                              // Trajectory start time (example: advanced trajectory started on 50% of motion time range)
    uint16_t   time_stop;                               // Trajectory stop time
    uint16_t   time_update;                             // Motion time value for motion configuration update
    uint16_t   time_step;                               // Trajectory time step (speed)
    uint8_t    flags;
} motion_config_t;


extern void motion_core_init(const point_3d_t* start_point_list);
extern void motion_core_start_motion(const motion_config_t* motion_config);
extern void motion_core_update_motion_config(const motion_config_t* motion_config);
extern void motion_core_process(void);
extern bool motion_core_is_motion_complete(void);


#endif /* _MOTION_CORE_H_ */
