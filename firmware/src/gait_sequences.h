//  ***************************************************************************
/// @file    gait_sequences.h
/// @author  NeoProg
/// @brief   Gait sequences
//  ***************************************************************************
#ifndef GAIT_SEQUENCES_H_
#define GAIT_SEQUENCES_H_

#include "motion_core.h"
#include "movement_engine.h"

#define LIMB_UP_Y                       (-50)
#define LIMB_DOWN_Y                     (-75)


typedef struct {
    bool     is_sequence_looped;
    uint32_t main_motions_begin;
    uint32_t finalize_motions_begin;
    uint32_t total_motions_count;
    motion_config_t motion_list[15];    // Sequence motion list
} sequence_info_t;


static const sequence_info_t sequence_down = {  
    
    .is_sequence_looped     = false,
    .main_motions_begin     = 0,
    .finalize_motions_begin = 1,
    .total_motions_count    = 1,
    
    {
        {
            {{-140, -25, 65}, {-150, -25, 0}, {-140, -25, -65}, {140, -25, 65}, {150, -25, 0}, {140, -25, -65}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .time_offset = 0, .time_step = 10, .time_for_config_update = MOTION_TIME_NO_UPDATE
        }
    }
};

static const sequence_info_t sequence_up = {

    .is_sequence_looped     = false,
    .main_motions_begin     = 0,
    .finalize_motions_begin = 5,
    .total_motions_count    = 5,
     
    {
        {
            {{-110, LIMB_DOWN_Y, 65}, {-130, LIMB_DOWN_Y, 0}, {-110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .time_offset = 0, .time_step = 25, .time_for_config_update = MOTION_TIME_NO_UPDATE
        },
        {    // Up 0, 2, 4 legs
            {{-110, LIMB_UP_Y, 65}, {-130, LIMB_DOWN_Y, 0}, {-110, LIMB_UP_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}}, 
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .time_offset = 0, .time_step = 25, .time_for_config_update = MOTION_TIME_NO_UPDATE
        },
        {    // Down 0, 2, 4 legs
            {{-110, LIMB_DOWN_Y, 65}, {-130, LIMB_DOWN_Y, 0}, {-110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .time_offset = 0, .time_step = 25, .time_for_config_update = MOTION_TIME_NO_UPDATE
        },
        {    // Up 1, 3, 5 legs
            {{-110, LIMB_DOWN_Y, 65}, {-130, LIMB_UP_Y, 0}, {-110, LIMB_DOWN_Y, -65}, {110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -65}}, 
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .time_offset = 0, .time_step = 25, .time_for_config_update = MOTION_TIME_NO_UPDATE
        },
        {   // Down 0, 2, 4 legs
            {{-110, LIMB_DOWN_Y, 65}, {-130, LIMB_DOWN_Y, 0}, {-110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}}, 
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .time_offset = 0, .time_step = 25, .time_for_config_update = MOTION_TIME_NO_UPDATE
        },
    }
};

#endif /* GAIT_SEQUENCES_H_ */