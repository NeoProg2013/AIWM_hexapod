//  ***************************************************************************
/// @file    motion_core.h
/// @author  NeoProg
/// @brief   Hexapod motion core
//  ***************************************************************************
#ifndef _MOTION_CORE_H_
#define _MOTION_CORE_H_
#include <stdint.h>
#include <stdbool.h>
#include "cli.h"

#define SUPPORT_LIMBS_COUNT                 (6)

#define MOTION_DEFAULT_SPEED                (87)
#define LIMB_STEP_HEIGHT                    (30)

#define MTIME_SCALE                         (1000u)
#define MTIME_MIN_VALUE                     (0u * MTIME_SCALE)
#define MTIME_MAX_VALUE                     (1u * MTIME_SCALE)
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

typedef struct {
    float x;
    float y;
    float z;
} point_3d_t;

typedef struct {
    point_3d_t   dest_positions[SUPPORT_LIMBS_COUNT];     // Destination point for each limb
    trajectory_t trajectories[SUPPORT_LIMBS_COUNT];       // Motion trajectory
    time_dir_t   time_directions[SUPPORT_LIMBS_COUNT];    // Motion time direction for each limb
    point_3d_t   start_positions[SUPPORT_LIMBS_COUNT];    // Initialize auto when motion started
    int32_t      motion_time;                             // Trajectory motion time. Can use for set start motion time value
    int32_t      time_stop;                               // Trajectory stop time
    int32_t      time_update;                             // Motion time value for motion configuration update
    int32_t      time_step;                               // Trajectory time step (speed)
} motion_t;

typedef struct {
    int32_t speed;
    int32_t curvature;
    int32_t distance;
} motion_config_t;


extern void motion_core_init(const point_3d_t* start_points);
extern void motion_core_start_motion(const motion_t* motion);
extern void motion_core_init_motion_config(const motion_config_t* motion_config);
extern void motion_core_update_motion_config(const motion_config_t* motion_config);
extern void motion_core_process(void);
extern bool motion_core_is_motion_complete(void);

extern bool motion_core_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response);


#endif /* _MOTION_CORE_H_ */
