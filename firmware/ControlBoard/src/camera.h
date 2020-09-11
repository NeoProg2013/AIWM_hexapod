//  ***************************************************************************
/// @file    camera.h
/// @author  NeoProg
/// @brief   Camera interface
//  ***************************************************************************
#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <stdint.h>
#include <stdbool.h>
#include "cli.h"


extern void camera_init(void);
extern void camera_process(void);
extern void camera_get_ip_address(uint8_t* buffer);

extern bool camera_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response);


#endif // _CAMERA_H_
