//  ***************************************************************************
/// @file    smcu.h
/// @author  NeoProg
/// @brief   SMCU communication driver
//  ***************************************************************************
#ifndef _SMCU_H_
#define _SMCU_H_
#include <stdint.h>
#include <stdbool.h>
#include "cli.h"


extern void smcu_init(void);
extern void smcu_process(void);
extern void smcu_get_sensor_data(int16_t** foot_sensors, int16_t** accel_sensor);

extern bool smcu_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response);


#endif // _SMCU_H_
