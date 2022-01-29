/// ***************************************************************************
/// @file    motion-scripts.h
/// @author  NeoProg
/// @brief   Hexapod motion scripts
/// ***************************************************************************
#ifndef _MOTION_SCRIPTS_H_
#define _MOTION_SCRIPTS_H_
#include "motion-core.h"

typedef struct {
    void(*init)(motion_t* motion);
    void(*exec)(motion_t* motion);
} motion_script_t;


extern motion_script_t motion_scripts[MOTION_SCRIPTS_COUNT];


#endif /* _MOTION_SCRIPTS_H_ */
