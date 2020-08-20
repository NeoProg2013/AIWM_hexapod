//  ***************************************************************************
/// @file    cli_core.h
/// @author  NeoProg
/// @brief   CLI core
//  ***************************************************************************
#ifndef _CLI_CORE_H_
#define _CLI_CORE_H_


extern void cli_core_init(void(*send_data)(const char*));
extern void cli_core_reset(void);
extern void cli_core_symbol_received(char symbol);


#endif // _CLI_CORE_H_

