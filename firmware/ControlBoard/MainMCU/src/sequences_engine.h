//  ***************************************************************************
/// @file    sequences_engine.h
/// @author  NeoProg
/// @brief   Hexapod sequences engine
//  ***************************************************************************
#ifndef _SEQUENCES_ENGINE_H_
#define _SEQUENCES_ENGINE_H_

#include <stdint.h>


typedef enum {
    SEQUENCE_NONE,
    SEQUENCE_UP,
    SEQUENCE_DOWN,
    SEQUENCE_MOVE,

    // Useless sequences
    SEQUENCE_UP_DOWN,
    SEQUENCE_PUSH_PULL,
    SEQUENCE_ATTACK_LEFT,
    SEQUENCE_ATTACK_RIGHT,
    SEQUENCE_DANCE,
    SEQUENCE_ROTATE_X,
    SEQUENCE_ROTATE_Z,
    
    SUPPORT_SEQUENCE_COUNT
} sequence_id_t;


extern void sequences_engine_init(void);
extern void sequences_engine_process(void);
extern void sequences_engine_select_sequence(sequence_id_t sequence, int32_t speed, int32_t curvature, int32_t distance);


#endif /* _SEQUENCES_ENGINE_H_ */
