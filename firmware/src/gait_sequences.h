//  ***************************************************************************
/// @file    gait_sequences.h
/// @author  NeoProg
/// @brief   Gait sequences
//  ***************************************************************************
#ifndef GAIT_SEQUENCES_H_
#define GAIT_SEQUENCES_H_

#include "limbs_driver.h"
#include "movement_engine.h"

#define LIMB_UP_Y                       (-50)
#define LIMB_DOWN_Y                     (-65)


typedef struct {
    point_3d_t   point_list[SUPPORT_LIMB_COUNT];
    path_type_t  path_list[SUPPORT_LIMB_COUNT];
    uint32_t     smooth_point_count;
} sequence_iteration_t;

typedef struct {
    
    bool     is_sequence_looped;
    uint32_t main_sequence_begin;
    uint32_t finalize_sequence_begin;

    uint32_t total_iteration_count;
    sequence_iteration_t iteration_list[15];    // Sequence iterations list
    
} sequence_info_t;


static const sequence_info_t sequence_down = {  
    
    .is_sequence_looped      = false,
    .main_sequence_begin     = 0,
    .finalize_sequence_begin = 1,
    .total_iteration_count   = 1,
    
    {
        { 
            {{135, -35, 70}, {150, -35, 0}, {135, -35, -70}, {135, -35, 70}, {150, -35, 0}, {135, -35, -70}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR }, 80
        },    
    }
};

static const sequence_info_t sequence_up = {

    .is_sequence_looped      = false,
    .main_sequence_begin     = 0,
    .finalize_sequence_begin = 5,
    .total_iteration_count   = 5,
     
    {
        {   // Down all legs
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}}, 
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 40
        },
        {    // Up 0, 2, 4 legs
            {{110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}}, 
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 60
        },
        {    // Down 0, 2, 4 legs
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 60
        },
        {    // Up 1, 3, 5 legs
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -65}}, 
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 60
        },
        {   // Down 0, 2, 4 legs
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}}, 
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 60
        },
    }
};

static const sequence_info_t sequence_direct_movement = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 0,
    .finalize_sequence_begin = 2,
    .total_iteration_count   = 4,
    
    {
        //
        // Main sequence
        //
        {
            {{130, LIMB_UP_Y, 110}, {130, LIMB_DOWN_Y, -45}, {130, LIMB_UP_Y,  -20}, {110, LIMB_DOWN_Y,  20}, {130, LIMB_UP_Y,  45}, {110, LIMB_DOWN_Y, -110}},
            { PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR}, 60
        },
        {
            {{110, LIMB_DOWN_Y,  20}, {130, LIMB_UP_Y,  45}, {110, LIMB_DOWN_Y, -110}, {130, LIMB_UP_Y, 110}, {130, LIMB_DOWN_Y, -45}, {130, LIMB_UP_Y,  -20}},
            { PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS}, 60
        },

        //
        // Finalize sequence
        //
        {
            {{130, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {130, LIMB_UP_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR}, 60
        },
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 60
        },
    }
};

static const sequence_info_t sequence_run = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 0,
    .finalize_sequence_begin = 2,
    .total_iteration_count   = 4,
     
    {      
        //
        // Main sequence
        //
        { 
            {{130, LIMB_UP_Y, 110}, {130, LIMB_DOWN_Y, -45}, {130, LIMB_UP_Y,  -20}, {110, LIMB_DOWN_Y,  20}, {130, LIMB_UP_Y,  45}, {110, LIMB_DOWN_Y, -110}},
            { PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR}, 40
        },    
        { 
            {{110, LIMB_DOWN_Y,  20}, {130, LIMB_UP_Y,  45}, {110, LIMB_DOWN_Y, -110}, {130, LIMB_UP_Y, 110}, {130, LIMB_DOWN_Y, -45}, {130, LIMB_UP_Y,  -20}}, 
            { PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS}, 40
        },    

        //
        // Finalize sequence
        //
        { 
            {{130, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {130, LIMB_UP_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}}, 
            { PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR}, 60
        },
        { 
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}}, 
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 60
        },
    }
};

static const sequence_info_t sequence_reverse_movement = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 0,
    .finalize_sequence_begin = 2,
    .total_iteration_count   = 4,
    
    {
        //
        // Main sequence
        //
        {
            {{110, LIMB_DOWN_Y, 110}, {130, LIMB_UP_Y, -45}, {110, LIMB_DOWN_Y, -20}, {130, LIMB_UP_Y,  20}, {130, LIMB_DOWN_Y,  45}, {130, LIMB_UP_Y, -110}},
            { PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS}, 50
        },
        {
            {{130, LIMB_UP_Y,  20}, {130, LIMB_DOWN_Y, 45}, {130, LIMB_UP_Y, -110}, {110, LIMB_DOWN_Y, 110}, {130, LIMB_UP_Y, -45}, {110, LIMB_DOWN_Y, -20}},
            { PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR}, 50
        },
        

        //
        // Finalize sequence
        //
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}, {130, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {130, LIMB_UP_Y, -65}},
            { PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS}, 40
        },
        { 
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}}, 
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 40
        }
    }
};

static const sequence_info_t sequence_shift_left = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 1,
    .finalize_sequence_begin = 3,
    .total_iteration_count   = 4,
    
    {
        //
        // Prepare sequence
        //
        {
            {{110, LIMB_DOWN_Y, 65}, {170, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {170, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 40
        },
         
        //
        // Main sequence
        //
        {
            {{150, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {150, LIMB_UP_Y, -65}, {150, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {150, LIMB_DOWN_Y, -65}},
            { PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR}, 40
        },
        {
            {{110, LIMB_DOWN_Y, 65}, {170, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_UP_Y, 65}, {170, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -65}},
            { PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS}, 40
        },

        //
        // Finalize sequence
        //
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR, PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR}, 40
        },
    }
};

static const sequence_info_t sequence_shift_right = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 1,
    .finalize_sequence_begin = 3,
    .total_iteration_count   = 4,
    
    {
        //
        // Prepare sequence
        //
        {
            {{110, LIMB_DOWN_Y, 65}, {170, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {170, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 40
        },
        
        //
        // Main sequence
        //
        {
            {{150, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {150, LIMB_DOWN_Y, -65}, {150, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {150, LIMB_UP_Y, -65}},
            { PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS}, 40
        },
        {
            {{110, LIMB_UP_Y, 65}, {170, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -65}, {110, LIMB_DOWN_Y, 65}, {170, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR}, 40
        },

        //
        // Finalize sequence
        //
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR, PATH_LINEAR, PATH_YZ_ELLIPTICAL_X_SINUS, PATH_LINEAR}, 40
        },
    }
};

static const sequence_info_t sequence_rotate_left = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 0,
    .finalize_sequence_begin = 2,
    .total_iteration_count   = 2,
    
    {
        {
            {{125, LIMB_UP_Y, 22}, {122, LIMB_DOWN_Y, 45}, {81, LIMB_UP_Y, -98}, {125, LIMB_DOWN_Y, 22}, {122, LIMB_UP_Y, 45}, {81, LIMB_DOWN_Y, -98}},
            { PATH_XZ_ARC_Y_SINUS, PATH_XZ_ARC_Y_LINEAR, PATH_XZ_ARC_Y_SINUS, PATH_XZ_ARC_Y_LINEAR, PATH_XZ_ARC_Y_SINUS, PATH_XZ_ARC_Y_LINEAR}, 50
        },
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y,  0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -65}},
            { PATH_XZ_ARC_Y_LINEAR, PATH_XZ_ARC_Y_SINUS, PATH_XZ_ARC_Y_LINEAR, PATH_XZ_ARC_Y_SINUS, PATH_XZ_ARC_Y_LINEAR, PATH_XZ_ARC_Y_SINUS}, 50
        },
    }
};

static const sequence_info_t sequence_rotate_right = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 0,
    .finalize_sequence_begin = 2,
    .total_iteration_count   = 2,
    
    {
        {
            {{80, LIMB_UP_Y, 100}, {122, LIMB_DOWN_Y, -45}, {125, LIMB_UP_Y, -23}, {80, LIMB_DOWN_Y, 100}, {122, LIMB_UP_Y, -45}, {125, LIMB_DOWN_Y, -23}},
            { PATH_XZ_ARC_Y_SINUS, PATH_XZ_ARC_Y_LINEAR, PATH_XZ_ARC_Y_SINUS, PATH_XZ_ARC_Y_LINEAR, PATH_XZ_ARC_Y_SINUS, PATH_XZ_ARC_Y_LINEAR}, 50
        },
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y,   0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y,   0}, {110, LIMB_UP_Y, -65}},
            { PATH_XZ_ARC_Y_LINEAR, PATH_XZ_ARC_Y_SINUS, PATH_XZ_ARC_Y_LINEAR, PATH_XZ_ARC_Y_SINUS, PATH_XZ_ARC_Y_LINEAR, PATH_XZ_ARC_Y_SINUS}, 50
        },
    }
};

static const sequence_info_t sequence_direct_movement_slow = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 0,
    .finalize_sequence_begin = 2,
    .total_iteration_count   = 4,
    
    {
        //
        // Main sequence
        //
        {
            {{130, LIMB_UP_Y, 105}, {130, LIMB_DOWN_Y, -20}, {130, LIMB_UP_Y,  -65}, {110, LIMB_DOWN_Y,  65}, {130, LIMB_UP_Y,  20}, {110, LIMB_DOWN_Y, -105}},
            { PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR}, 50
        },
        {
            {{110, LIMB_DOWN_Y,  65}, {130, LIMB_UP_Y,  20}, {110, LIMB_DOWN_Y, -105}, {130, LIMB_UP_Y, 105}, {130, LIMB_DOWN_Y, -20}, {130, LIMB_UP_Y,  -65}},
            { PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS}, 50
        },

        //
        // Finalize sequence
        //
        {
            {{130, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {130, LIMB_UP_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR}, 40
        },
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 40
        },
    }
};

static const sequence_info_t sequence_reverse_movement_slow = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 0,
    .finalize_sequence_begin = 2,
    .total_iteration_count   = 4,
    
    {
        //
        // Main sequence
        //
        {
            {{110, LIMB_DOWN_Y, 105}, {130, LIMB_UP_Y, -20}, {110, LIMB_DOWN_Y, -65}, {130, LIMB_UP_Y,  65}, {130, LIMB_DOWN_Y,  20}, {130, LIMB_UP_Y, -105}},
            { PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS}, 50
        },
        {
            {{130, LIMB_UP_Y,  65}, {130, LIMB_DOWN_Y, 20}, {130, LIMB_UP_Y, -105}, {110, LIMB_DOWN_Y, 105}, {130, LIMB_UP_Y, -20}, {110, LIMB_DOWN_Y, -65}},
            { PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR}, 50
        },
        

        //
        // Finalize sequence
        //
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}, {130, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {130, LIMB_UP_Y, -65}},
            { PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS, PATH_LINEAR, PATH_XZ_ELLIPTICAL_Y_SINUS}, 40
        },
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 40
        }
    }
};

static const sequence_info_t sequence_attack_left = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 1,
    .finalize_sequence_begin = 3,
    .total_iteration_count   = 4,
    
    {
        //
        // Prepare sequence
        //
        {
            {{0, 0, 150}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 50
        },
        
        //
        // Main sequence
        //
        {
            {{0, 50, 250}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 50
        },
        {
            {{0,  0, 150}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 50
        },

        //
        // Finalize sequence
        //
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 50
        },
    }
};

static const sequence_info_t sequence_attack_right = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 1,
    .finalize_sequence_begin = 3,
    .total_iteration_count   = 4,
    
    {
        //
        // Prepare sequence
        //
        {
            { {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {0, 0, 150}, {130, LIMB_DOWN_Y, 0}, { 110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 50
        },
        
        //
        // Main sequence
        //
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {0, 50, 250}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 50
        },
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {0, 0, 150}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 50
        },

        //
        // Finalize sequence
        //
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 50
        },
    }
};

static const sequence_info_t sequence_dance = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 0,
    .finalize_sequence_begin = 12,
    .total_iteration_count   = 12,
    
    {
        //
        // Main sequence
        //
        {    // Up 0, 2, 4 legs
            {{110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        {    // Down 0, 2, 4 legs
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        {    // Up 1, 3, 5 legs
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        {   // Down 0, 2, 4 legs
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        
        {
            {{170, 50, 170}, {130, LIMB_DOWN_Y, 0}, {170, 50, -170}, {110, LIMB_DOWN_Y, 65}, {240, 50, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 70
        },
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 70
        },
        {
            {{110, LIMB_DOWN_Y, 65}, {240,  50, 0}, {110, LIMB_DOWN_Y, -65}, {170, 50, 170}, {130, LIMB_DOWN_Y, 0}, {170, 50, -170}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 70
        },
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 70
        },
        
        {    // Up 0, 2, 4 legs
 
            {{110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        {    // Down 0, 2, 4 legs
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        {    // Up 1, 3, 5 legs
 
            {{110, LIMB_DOWN_Y, 65}, {135, LIMB_UP_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        {   // Down 0, 2, 4 legs
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        }
    }
};

/*
static const sequence_info_t sequence_rotate_x = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 4,
    .finalize_sequence_begin = 8,
    .total_iteration_count   = 13,
    
    {
        //
        // Prepare sequence
        //
        {   // Up 0 leg. Move to start point
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        {   // Down 0 leg
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, LIMB_DOWN_Y, 102}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        {   // Up 3 leg. Move to start point
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, LIMB_DOWN_Y, 102}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        {   // Down 3 leg
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, LIMB_DOWN_Y, 102}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 102}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        
        //
        // Main sequence
        //
        {
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, -155, 102}, {130, LIMB_UP_Y, 0}, {110, -65, -65}, {110, -155, 102}, {130, LIMB_UP_Y, 0}, {110, -65, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 90
        },
        {
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, -110, 102}, {130, LIMB_UP_Y, 0}, {110, -110, -65}, {110, -110, 102}, {130, LIMB_UP_Y, 0}, {110, -110, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 90
        },
        {
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, -65, 65}, {130, LIMB_UP_Y, 0}, {110, -155, -102}, {110, -65, 65}, {130, LIMB_UP_Y, 0}, {110, -155, -102}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 90
        },
        {
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, -110, 65}, {130, LIMB_UP_Y, 0}, {110, -110, -102}, {110, -110, 65}, {130, LIMB_UP_Y, 0}, {110, -110, -102}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 90
        },
        
        //
        // Finalize sequence
        //
        {   // Go to default height
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -102}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -102}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 50
        },
        {   // Up 2 leg. Move to start point
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -102}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -102}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        {   // Down 2 leg
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -102}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        {   // Up 5 leg. Move to start point
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -102}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
        {   // Down 5 leg
            { LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN, LIMB_STATE_DOWN },
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 30
        },
    }
};*/

static const sequence_info_t sequence_rotate_x = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 1,
    .finalize_sequence_begin = 3,
    .total_iteration_count   = 4,
    
    {
        //
        // Prepare sequence
        //
        {
            {{110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, -110, -65}, {110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, -110, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 45
        },
        
        //
        // Main sequence
        //
        {
            {{110, -110, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -65}, {110, -110, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_UP_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 90
        },
        {
            {{110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, -110, -65}, {110, LIMB_UP_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, -110, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 90
        },

        
        //
        // Finalize sequence
        //
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 45
        },
    }
};

static const sequence_info_t sequence_rotate_z = {

    .is_sequence_looped      = true,
    .main_sequence_begin     = 1,
    .finalize_sequence_begin = 3,
    .total_iteration_count   = 4,
    
    {
        //
        // Prepare sequence
        //
        {
            {{110, LIMB_UP_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_UP_Y, -65}, {110, -110, 65}, {130, -110, 0}, {110, -110, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 45
        },
        
        //
        // Main sequence
        //
        {
            {{110, -110, 65}, {130, -110, 0}, {110, -110, -65}, {110, LIMB_UP_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_UP_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 90
        },
        {
            {{110, LIMB_UP_Y, 65}, {130, LIMB_UP_Y, 0}, {110, LIMB_UP_Y, -65}, {110, -110, 65}, {130, -110, 0}, {110, -110, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 90
        },

        
        //
        // Finalize sequence
        //
        {
            {{110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}, {110, LIMB_DOWN_Y, 65}, {130, LIMB_DOWN_Y, 0}, {110, LIMB_DOWN_Y, -65}},
            { PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR, PATH_LINEAR}, 45
        },
    }
};


#endif /* GAIT_SEQUENCES_H_ */