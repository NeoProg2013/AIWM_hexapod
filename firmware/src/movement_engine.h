//  ***************************************************************************
/// @file    movement_engine.h
/// @author  NeoProg
/// @brief   Hexapod movement engine
//  ***************************************************************************
#ifndef MOVEMENT_ENGINE_H_
#define MOVEMENT_ENGINE_H_


typedef enum {
    SEQUENCE_NONE,
    
    SEQUENCE_UPDATE_HEIGHT,
    SEQUENCE_UP,
    SEQUENCE_DOWN,
    
    SEQUENCE_RUN,
    SEQUENCE_DIRECT_MOVEMENT,
    SEQUENCE_REVERSE_MOVEMENT,
    SEQUENCE_ROTATE_LEFT,
    SEQUENCE_ROTATE_RIGHT,
    
    SEQUENCE_DIRECT_MOVEMENT_SLOW,
    SEQUENCE_REVERSE_MOVEMENT_SLOW,
    
    SEQUENCE_SHIFT_LEFT,
    SEQUENCE_SHIFT_RIGHT,
    
    SEQUENCE_ATTACK_LEFT,
    SEQUENCE_ATTACK_RIGHT,
    SEQUENCE_DANCE,
    SEQUENCE_ROTATE_X,
    //SEQUENCE_ROTATE_Y,
    SEQUENCE_ROTATE_Z,
    
    SUPPORT_SEQUENCE_COUNT
} sequence_id_t;


extern void movement_engine_init(void);
extern void movement_engine_process(void);
extern void movement_engine_increase_height(void);
extern void movement_engine_decrease_height(void);
extern void movement_engine_select_sequence(sequence_id_t sequence);


#endif /* MOVEMENT_ENGINE_H_ */