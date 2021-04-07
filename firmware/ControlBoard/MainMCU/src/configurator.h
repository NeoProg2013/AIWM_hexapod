//  ***************************************************************************
/// @file    configurator.c
/// @author  NeoProg
/// @brief   System configuration interface
//  ***************************************************************************
#ifndef _CONFIGURATOR_H_
#define _CONFIGURATOR_H_

#include <stdint.h>
#include <stdbool.h>
#include "memory_map.h"
#include "cli.h"


extern void config_init(void);
extern bool config_check_intergity(void);
extern bool config_read(uint32_t address, uint8_t* buffer, uint32_t bytes_count);
extern bool config_write(uint32_t address, uint8_t* data, uint32_t bytes_count);

extern bool config_read_8(uint32_t address, uint8_t* buffer);
extern bool config_read_16(uint32_t address, uint16_t* buffer);
extern bool config_read_32(uint32_t address, uint32_t* buffer);

extern bool config_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response);


#endif // _CONFIGURATOR_H_
