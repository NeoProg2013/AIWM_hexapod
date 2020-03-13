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


typedef struct {
    float x;
    float y;
    float z;
} point_3d_t;


extern void motion_core_init(const point_3d_t* point_list);
extern void motion_core_process(void);

extern bool motion_core_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], 
									        uint32_t argc, char* response);


#endif /* _MOTION_CORE_H_ */