//  ***************************************************************************
/// @file    cli.h
/// @author  NeoProg
/// @brief   Command line interface driver
//  ***************************************************************************
#ifndef _CLI_H_
#define _CLI_H_

#include <stdint.h>

#define CLI_MSG(msg)            "----> " msg "\r\n"

#define CLI_ARG_COUNT           (3)
#define CLI_ARG_MAX_SIZE        (64)


extern void cli_init(void);
extern uint32_t cli_process_frame(char* rx_buffer, char* tx_buffer);


#endif // _CLI_H_
