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
extern void cli_process(void);


#endif // _CLI_H_
