//  ***************************************************************************
/// @file    movement_engine.h
/// @author  NeoProg
/// @brief   Hexapod movement engine
//  ***************************************************************************
#ifndef MOVEMENT_ENGINE_H_
#define MOVEMENT_ENGINE_H_

#include <stdint.h>


typedef enum {
    SEQUENCE_NONE,
    SEQUENCE_UP,
    SEQUENCE_DOWN,
    SEQUENCE_DIRECT,
    SEQUENCE_REVERSE,
    
    SUPPORT_SEQUENCE_COUNT
} sequence_id_t;


extern void movement_engine_init(void);
extern void movement_engine_process(void);
extern void movement_engine_select_sequence(sequence_id_t sequence);


#endif /* MOVEMENT_ENGINE_H_ */