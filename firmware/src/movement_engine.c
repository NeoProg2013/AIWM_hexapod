//  ***************************************************************************
/// @file    movement_engine.c
/// @author  NeoProg
//  ***************************************************************************
#include "movement_engine.h"
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
    STATE_MOVE,             // Process step of current gait
    STATE_WAIT,             // Wait limbs movement complete
    STATE_NEXT_ITERATION,   // Select next step of current gait
    STATE_CHANGE_SEQUENCE   // Change current sequence (if needed)
} driver_state_t;

typedef enum {
    SEQUENCE_STAGE_PREPARE,
    SEQUENCE_STAGE_MAIN,
    SEQUENCE_STAGE_FINALIZE
} sequence_stage_t;

typedef enum {
    HEXAPOD_STATE_DOWN,
    HEXAPOD_STATE_UP
} hexapod_state_t;


static driver_state_t driver_state = STATE_NOINIT;
static hexapod_state_t hexapod_state = HEXAPOD_STATE_DOWN;

static sequence_id_t current_sequence = SEQUENCE_NONE;
static const sequence_info_t* current_sequence_info = NULL;

static sequence_id_t next_sequence = SEQUENCE_NONE;
static const sequence_info_t* next_sequence_info = NULL;


//  ***************************************************************************
/// @brief  Movement driver initialization
/// @param  none
/// @return none
//  ***************************************************************************
void movement_engine_init(void) {

    // Select DOWN sequence as start position
    current_sequence      = SEQUENCE_DOWN;
    current_sequence_info = &sequence_down;
    next_sequence         = SEQUENCE_DOWN;
    next_sequence_info    = &sequence_down;
    hexapod_state         = HEXAPOD_STATE_DOWN;
    
    // Initialize limbs driver
    uint32_t last_motion_index = sequence_down.total_motions_count - 1;
    motion_core_init(sequence_down.motion_list[last_motion_index].dest_positions);
    
    // Initialization driver state
    driver_state = STATE_IDLE;
}

void movement_engine_set_motion_config(uint32_t step_length, uint32_t curvature) {
    
}

//  ***************************************************************************
/// @brief  Movement driver process
/// @param  none
/// @return none
//  ***************************************************************************
void movement_engine_process(void) {
    
    if (sysmon_is_module_disable(SYSMON_MODULE_MOVEMENT_ENGINE) == true) return; // Module disabled
    

    static sequence_stage_t sequence_stage = SEQUENCE_STAGE_PREPARE;
    static uint32_t current_motion = 0;

    switch (driver_state) {
        
        case STATE_IDLE:
            if (current_sequence != next_sequence) {
                driver_state = STATE_CHANGE_SEQUENCE;
            }
            break;
        
        case STATE_MOVE:
            for (uint32_t i = 0; i < SUPPORT_LIMBS_COUNT; ++i) {
                motion_core_start_motion(&current_sequence_info->motion_list[current_motion]);
            }
            driver_state = STATE_WAIT;
            break;
        
        case STATE_WAIT:
            if (motion_core_is_motion_complete() == true) {
                driver_state = STATE_NEXT_ITERATION;
            }
            break;
            
        case STATE_NEXT_ITERATION:
            
            ++current_motion;
            driver_state = STATE_MOVE;
            
            if (sequence_stage == SEQUENCE_STAGE_PREPARE && current_motion >= current_sequence_info->main_motions_begin) {
                sequence_stage = SEQUENCE_STAGE_MAIN;
            }
            if (sequence_stage == SEQUENCE_STAGE_MAIN && current_motion >= current_sequence_info->finalize_motions_begin) {
                
                if (current_sequence != next_sequence) { 
                    
                    // Need change current sequence - go to finalize motions if it available
                    current_motion = current_sequence_info->finalize_motions_begin;
                    sequence_stage = SEQUENCE_STAGE_FINALIZE;
                }
                else {
                    
                    if (current_sequence_info->is_sequence_looped == true) {
                        current_motion = current_sequence_info->main_motions_begin;
                    }
                    else {
                        // Current sequence completed and new sequence not selected
                        hexapod_state = (current_sequence == SEQUENCE_DOWN) ? HEXAPOD_STATE_DOWN : HEXAPOD_STATE_UP;
                        movement_engine_select_sequence(SEQUENCE_NONE);
                        driver_state = STATE_CHANGE_SEQUENCE;
                    }
                }              
            }
            if (sequence_stage == SEQUENCE_STAGE_FINALIZE && current_motion >= current_sequence_info->total_motions_count) {
                driver_state = STATE_CHANGE_SEQUENCE;
            }           
            break;
            
        case STATE_CHANGE_SEQUENCE:
            current_sequence      = next_sequence;
            current_sequence_info = next_sequence_info;
            current_motion        = 0;
            sequence_stage        = SEQUENCE_STAGE_PREPARE;
            driver_state          = STATE_MOVE;
            
            if (current_sequence == SEQUENCE_NONE) {
                driver_state = STATE_IDLE;
            }        
            break;
            
        case STATE_NOINIT:
        default:
            sysmon_set_error(SYSMON_FATAL_ERROR);
            sysmon_disable_module(SYSMON_MODULE_MOVEMENT_ENGINE);
            break;
    }
}

//  ***************************************************************************
/// @brief  Select sequence
/// @param  sequence: new sequence
/// @return none
//  ***************************************************************************
void movement_engine_select_sequence(sequence_id_t sequence) {
    
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
            }
            break;

        case SEQUENCE_REVERSE: 
            if (hexapod_state == HEXAPOD_STATE_UP) {
                next_sequence = SEQUENCE_REVERSE;
                next_sequence_info = &sequence_reverse;
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
            sysmon_disable_module(SYSMON_MODULE_MOVEMENT_ENGINE);
            return;
    }
}
