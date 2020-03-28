//  ***************************************************************************
/// @file    gait_sequences.h
/// @author  NeoProg
/// @brief   Gait sequences
//  ***************************************************************************
#ifndef GAIT_SEQUENCES_H_
#define GAIT_SEQUENCES_H_

#include "motion_core.h"


#define LIMB_UP_Y                       (LIMB_DOWN_Y + LIMB_STEP_HEIGHT)
#define LIMB_DOWN_Y                     (-80)


typedef struct {
    bool     is_sequence_looped;
    uint32_t main_motions_begin;
    uint32_t finalize_motions_begin;
    uint32_t total_motions_count;
    motion_config_t motion_list[15]; // Sequence motion list
} sequence_info_t;


static const sequence_info_t sequence_down = {  
    .is_sequence_looped     = false,
    .main_motions_begin     = 0,
    .finalize_motions_begin = 1,
    .total_motions_count    = 1,
    
    {
        {
            {{-140, -25, 83}, {-150, -25, 0}, {-140, -25, -83}, {140, -25, 83}, {150, -25, 0}, {140, -25, -83}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 5, .is_need_init_start_position = true
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
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 20, .is_need_init_start_position = true
        },
        {    // Up 0, 2, 4 legs
            {{-115, LIMB_UP_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_UP_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_UP_Y, 0}, {115, LIMB_DOWN_Y, -70}}, 
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 20, .is_need_init_start_position = true
        },
        {    // Down 0, 2, 4 legs
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 20, .is_need_init_start_position = true
        },
        {    // Up 1, 3, 5 legs
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_UP_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_UP_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_UP_Y, -70}}, 
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 20, .is_need_init_start_position = true
        },
        {   // Down 0, 2, 4 legs
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}}, 
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 20, .is_need_init_start_position = true
        },
    }
};

static const sequence_info_t sequence_direct = {
    .is_sequence_looped     = true,
    .main_motions_begin     = 2,
    .finalize_motions_begin = 4,
    .total_motions_count    = 6,
     
    {
        //
        // Prepare sequence
        //
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_UP_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_UP_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_UP_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            {0}, // Start points initialize after motion start
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 25, .is_need_init_start_position = true
        },
        {
            {0}, // Destination points is not use for TRAJECTORY_XZ_ADV_Y_CONST
            { TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS},
            { TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE },
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            .motion_time = MTIME_MID_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = false
        },
        
        //
        // Main sequence
        //
        {
            {0}, // Destination points is not use for TRAJECTORY_XZ_ADV_Y_CONST
            { TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST},
            { TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT },
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_MID_VALUE, .time_step = 10, .is_need_init_start_position = false
        },
        {
            {0}, // Destination points is not use for TRAJECTORY_XZ_ADV_Y_CONST
            { TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS},
            { TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE },
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_MID_VALUE, .time_step = 10, .is_need_init_start_position = false
        },
        
        //
        // Finalize sequence
        //
        {
            {0}, // Destination points is not use for TRAJECTORY_XZ_ADV_Y_CONST
            { TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST},
            { TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT },
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MID_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = false
        },
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            {0}, // Start points initialize after motion start
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 25, .is_need_init_start_position = true
        },
    }
};

static const sequence_info_t sequence_reverse = {
    .is_sequence_looped     = true,
    .main_motions_begin     = 2,
    .finalize_motions_begin = 4,
    .total_motions_count    = 6,
     
    {
        //
        // Prepare sequence
        //
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_UP_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_UP_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_UP_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            {0}, // Start points initialize after motion start
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 25, .is_need_init_start_position = true
        },
        {
            {0}, // Destination points is not use for TRAJECTORY_XZ_ADV_Y_CONST
            { TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS},
            { TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT },
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            .motion_time = MTIME_MID_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = false
        },
        
        //
        // Main sequence
        //
        {
            {0}, // Destination points is not use for TRAJECTORY_XZ_ADV_Y_CONST
            { TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST},
            { TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE },
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_MID_VALUE, .time_step = 10, .is_need_init_start_position = false
        },
        {
            {0}, // Destination points is not use for TRAJECTORY_XZ_ADV_Y_CONST
            { TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS},
            { TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT },
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_MID_VALUE, .time_step = 10, .is_need_init_start_position = false
        },
        
        //
        // Finalize sequence
        //
        {
            {0}, // Destination points is not use for TRAJECTORY_XZ_ADV_Y_CONST
            { TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_SINUS, TRAJECTORY_XZ_ADV_Y_CONST},
            { TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE, TIME_DIR_DIRECT, TIME_DIR_REVERSE },
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MID_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = false
        },
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            {0}, // Start points initialize after motion start
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 25, .is_need_init_start_position = true
        },
    }
};



static const sequence_info_t sequence_up_down = {
    .is_sequence_looped     = true,
    .main_motions_begin     = 0,
    .finalize_motions_begin = 2,
    .total_motions_count    = 2,
     
    {
        {
            {{-115, LIMB_DOWN_Y - 50, 70}, {-135, LIMB_DOWN_Y - 50, 0}, {-115, LIMB_DOWN_Y - 50, -70}, {115, LIMB_DOWN_Y - 50, 70}, {135, LIMB_DOWN_Y - 50, 0}, {115, LIMB_DOWN_Y - 50, -70}}, 
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        }
    }
};

static const sequence_info_t sequence_push_pull = {
    .is_sequence_looped     = true,
    .main_motions_begin     = 1,
    .finalize_motions_begin = 3,
    .total_motions_count    = 4,
     
    {
        //
        // Prepare sequence
        //
        {
            {0}, // Destination points is not use for TRAJECTORY_XZ_ADV_Y_CONST
            { TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            .motion_time = MTIME_MID_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 5, .is_need_init_start_position = false
        },
        
        //
        // Main sequence
        //
        {
            {0}, // Destination points is not use for TRAJECTORY_XZ_ADV_Y_CONST
            { TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST},
            { TIME_DIR_REVERSE, TIME_DIR_REVERSE, TIME_DIR_REVERSE, TIME_DIR_REVERSE, TIME_DIR_REVERSE, TIME_DIR_REVERSE },
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 5, .is_need_init_start_position = false
        },
        {
            {0}, // Destination points is not use for TRAJECTORY_XZ_ADV_Y_CONST
            { TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 5, .is_need_init_start_position = false
        },
        
        //
        // Finalize sequence
        //
        {
            {0}, // Destination points is not use for TRAJECTORY_XZ_ADV_Y_CONST
            { TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST, TRAJECTORY_XZ_ADV_Y_CONST},
            { TIME_DIR_REVERSE, TIME_DIR_REVERSE, TIME_DIR_REVERSE, TIME_DIR_REVERSE, TIME_DIR_REVERSE, TIME_DIR_REVERSE },
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MID_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 5, .is_need_init_start_position = false
        }
    }
};

static const sequence_info_t sequence_attack_left = {
    .is_sequence_looped     = true,
    .main_motions_begin     = 1,
    .finalize_motions_begin = 3,
    .total_motions_count    = 4,
    
    {
        //
        // Prepare sequence
        //
        {
            {{0, 0, 150}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },
        
        //
        // Main sequence
        //
        {
            {{0, 0, 250}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },
        {
            {{0, 0, 150}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },

        //
        // Finalize sequence
        //
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },
    }
};

static const sequence_info_t sequence_attack_right = {
    .is_sequence_looped     = true,
    .main_motions_begin     = 1,
    .finalize_motions_begin = 3,
    .total_motions_count    = 4,
    
    {
        //
        // Prepare sequence
        //
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {0, 0, 150}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },
        
        //
        // Main sequence
        //
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {0, 0, 250}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {0, 0, 150}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },

        //
        // Finalize sequence
        //
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },
    }
};

/*static const sequence_info_t sequence_attack = {

    .is_sequence_looped     = true,
    .main_motions_begin     = 1,
    .finalize_motions_begin = 3,
    .total_motions_count    = 4,
    
    {
        //
        // Prepare sequence
        //
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {0, 0, 150}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },
        
        //
        // Main sequence
        //
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {0, 0, 250}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {0, 0, 150}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },

        //
        // Finalize sequence
        //
        {
            {{-115, LIMB_DOWN_Y, 70}, {-135, LIMB_DOWN_Y, 0}, {-115, LIMB_DOWN_Y, -70}, {115, LIMB_DOWN_Y, 70}, {135, LIMB_DOWN_Y, 0}, {115, LIMB_DOWN_Y, -70}},
            { TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR, TRAJECTORY_XYZ_LINEAR},
            { TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT, TIME_DIR_DIRECT },
            .motion_time = MTIME_MIN_VALUE, .time_stop = MTIME_MAX_VALUE, .time_update = MTIME_NO_UPDATE, .time_step = 10, .is_need_init_start_position = true
        },
    }
};*/

#endif /* GAIT_SEQUENCES_H_ */