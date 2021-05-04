//  ***************************************************************************
/// @file    smcu.h
/// @author  NeoProg
/// @brief   SMCU communication driver
//  ***************************************************************************
#ifndef _SMCU_H_
#define _SMCU_H_

#include "cli.h"
#include <stdbool.h>


extern void smcu_init(void);
extern void smcu_process(void);
extern bool smcu_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response);


#endif // _SMCU_H_
