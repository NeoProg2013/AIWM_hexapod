//  ***************************************************************************
/// @file    sequences_engine.c
/// @author  NeoProg
//  ***************************************************************************
#include "sequences_engine.h"
#include "motion_core.h"
#include "gait_sequences.h"
#include "system_monitor.h"
#include "systimer.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


typedef enum {
    STATE_NOINIT,           // Module not initialized
    STATE_IDLE,
    STATE_MOVE,             // Process motion of current sequence
    STATE_WAIT,             // Wait motion complete
    STATE_NEXT_MOTION,      // Select next motion of current sequence
    STATE_CHANGE_SEQUENCE   // Change current sequence (if needed)
} engine_state_t;

typedef enum {
    STAGE_PREPARE,
    STAGE_MAIN,
    STAGE_FINALIZE
} stage_t;

typedef enum {
    HEXAPOD_STATE_DOWN,
    HEXAPOD_STATE_UP
} hexapod_state_t;


static engine_state_t engine_state = STATE_NOINIT;
static hexapod_state_t hexapod_state = HEXAPOD_STATE_DOWN;

static sequence_id_t current_sequence = SEQUENCE_NONE;
static const sequence_info_t* current_sequence_info = NULL;

static sequence_id_t next_sequence = SEQUENCE_NONE;
static const sequence_info_t* next_sequence_info = NULL;


//  ***************************************************************************
/// @brief  Sequences engine initialization
/// @param  none
/// @return none
//  ***************************************************************************
void sequences_engine_init(void) {

    // Select DOWN sequence as start position
    current_sequence      = SEQUENCE_DOWN;
    current_sequence_info = &sequence_down;
    next_sequence         = SEQUENCE_DOWN;
    next_sequence_info    = &sequence_down;
    hexapod_state         = HEXAPOD_STATE_DOWN;
    
    // Initialize motion driver
    uint32_t last_motion_index = sequence_down.total_motions_count - 1;
    motion_core_init(sequence_down.motion_list[last_motion_index].dest_positions);
    
    // Initialization engine state
    engine_state = STATE_IDLE;
}

//  ***************************************************************************
/// @brief  Sequences engine process
/// @param  none
/// @return none
//  ***************************************************************************
void sequences_engine_process(void) {
    
    if (sysmon_is_module_disable(SYSMON_MODULE_SEQUENCES_ENGINE) == true) return; // Module disabled
    

    static stage_t sequence_stage = STAGE_PREPARE;
    static uint32_t current_motion = 0;

    switch (engine_state) {
        
        case STATE_IDLE:
            if (current_sequence != next_sequence) {
                engine_state = STATE_CHANGE_SEQUENCE;
            }
            break;
        
        case STATE_MOVE:
            for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
                motion_core_start_motion(&current_sequence_info->motion_list[current_motion]);
            }
            engine_state = STATE_WAIT;
            break;
        
        case STATE_WAIT:
            if (motion_core_is_motion_complete() == true) {
                engine_state = STATE_NEXT_MOTION;
            }
            break;
            
        case STATE_NEXT_MOTION:
            ++current_motion;
            engine_state = STATE_MOVE;
            
            if (sequence_stage == STAGE_PREPARE && current_motion >= current_sequence_info->main_motions_begin) {
                sequence_stage = STAGE_MAIN;
            }
            if (sequence_stage == STAGE_MAIN && current_motion >= current_sequence_info->finalize_motions_begin) {
                
                if (current_sequence != next_sequence) { 
                    // Need change current sequence - go to finalize motions if it available
                    current_motion = current_sequence_info->finalize_motions_begin;
                    sequence_stage = STAGE_FINALIZE;
                }
                else {
                    
                    if (current_sequence_info->is_sequence_looped == true) {
                        current_motion = current_sequence_info->main_motions_begin;
                    }
                    else {
                        // Not looped sequence completed and new sequence not selected
                        sequences_engine_select_sequence(SEQUENCE_NONE, 0, 0);
                        engine_state = STATE_CHANGE_SEQUENCE;
                    }
                }              
            }
            if (sequence_stage == STAGE_FINALIZE && current_motion >= current_sequence_info->total_motions_count) {
                engine_state = STATE_CHANGE_SEQUENCE;
            }    
            
            // Change hexapod state
            hexapod_state = (current_sequence == SEQUENCE_DOWN) ? HEXAPOD_STATE_DOWN : HEXAPOD_STATE_UP;
            break;
            
        case STATE_CHANGE_SEQUENCE:
            current_sequence      = next_sequence;
            current_sequence_info = next_sequence_info;
            current_motion        = 0;
            sequence_stage        = STAGE_PREPARE;
            engine_state          = STATE_MOVE;
            
            motion_core_reset_trajectory_config();
            
            if (current_sequence == SEQUENCE_NONE) {
                engine_state = STATE_IDLE;
            }        
            break;
            
        case STATE_NOINIT:
        default:
            sysmon_set_error(SYSMON_FATAL_ERROR);
            sysmon_disable_module(SYSMON_MODULE_SEQUENCES_ENGINE);
            break;
    }
}

//  ***************************************************************************
/// @brief  Select sequence
/// @param  sequence: new sequence
/// @param  curvature: curvature value for DIRECT\REVERSE sequences
/// @param  step_length: step length value for DIRECT\REVERSE sequences
/// @return none
//  ***************************************************************************
void sequences_engine_select_sequence(sequence_id_t sequence, int32_t curvature, int32_t step_length) {
    
    // Request switch current sequence
    switch (sequence) {
        
        case SEQUENCE_NONE:
            next_sequence = SEQUENCE_NONE;
            next_sequence_info = NULL;
            break;
        
        case SEQUENCE_UP:
            if (hexapod_state == HEXAPOD_STATE_DOWN) {
                next_sequence = SEQUENCE_UP;
                next_sequence_info = &sequence_up;
            }
            break;

        case SEQUENCE_DOWN:
            next_sequence = SEQUENCE_DOWN;
            next_sequence_info = &sequence_down;
            break;
            
        /*case SEQUENCE_RUN:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_RUN;
                next_sequence_info = &sequence_run;
            }
            break;*/

        case SEQUENCE_DIRECT:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_DIRECT;
                next_sequence_info = &sequence_direct;
                motion_core_update_trajectory_config(curvature, step_length);
            }
            break;

        case SEQUENCE_REVERSE: 
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_REVERSE;
                next_sequence_info = &sequence_reverse;
                motion_core_update_trajectory_config(curvature, step_length);
            }
            break;

        /*ase SEQUENCE_ROTATE_LEFT:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_ROTATE_LEFT;
                next_sequence_info = &sequence_rotate_left;
            }
            break;

        case SEQUENCE_ROTATE_RIGHT:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_ROTATE_RIGHT;
                next_sequence_info = &sequence_rotate_right;
            }
            break;
        
        case SEQUENCE_DIRECT_MOVEMENT_SLOW:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_DIRECT_MOVEMENT_SLOW;
                next_sequence_info = &sequence_direct_movement_slow;
            }
            break;

        case SEQUENCE_REVERSE_MOVEMENT_SLOW:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_REVERSE_MOVEMENT_SLOW;
                next_sequence_info = &sequence_reverse_movement_slow;
            }
            break;

        case SEQUENCE_SHIFT_LEFT:
             if (hexapod_state == HEXAPOD_STATE_UP) {
                 next_sequence = SEQUENCE_SHIFT_LEFT;
                 next_sequence_info = &sequence_shift_left;
             }
             break;
             
        case SEQUENCE_SHIFT_RIGHT:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_SHIFT_RIGHT;
                next_sequence_info = &sequence_shift_right;
            }
            break;
 
        case SEQUENCE_ATTACK_LEFT:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_ATTACK_LEFT;
                next_sequence_info = &sequence_attack_left;
            }
            break;
            
        case SEQUENCE_ATTACK_RIGHT:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_ATTACK_RIGHT;
                next_sequence_info = &sequence_attack_right;
            }
            break;
            
        case SEQUENCE_DANCE:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_DANCE;
                next_sequence_info = &sequence_dance;
            }
            break;
            
        case SEQUENCE_ROTATE_X:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_ROTATE_X;
                next_sequence_info = &sequence_rotate_x;
            }
            break;*/
            
        /*case SEQUENCE_ROTATE_Y:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_ROTATE_Y;
                next_sequence_info = &sequence_rotate_y;
            }
            break;*/
            
        /*case SEQUENCE_ROTATE_Z:
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_ROTATE_Z;
                next_sequence_info = &sequence_rotate_z;
            }
            break;*/

        default:
            sysmon_set_error(SYSMON_FATAL_ERROR);
            sysmon_disable_module(SYSMON_MODULE_SEQUENCES_ENGINE);
            return;
    }
}
