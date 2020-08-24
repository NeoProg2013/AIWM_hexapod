//  ***************************************************************************
/// @file    cli.h
/// @author  NeoProg
/// @brief   Command line interface driver
//  ***************************************************************************
#ifndef _CLI_H_
#define _CLI_H_

#include <stdint.h>

#define CLI_MSG(msg)            "> " msg "\r\n" CLI_COLOR_RESET
#define CLI_HELP(msg)           "> " CLI_COLOR_YELLOW msg "\r\n" CLI_COLOR_RESET
#define CLI_OK(msg)             "> " CLI_COLOR_GREEN  msg "\r\n" CLI_COLOR_RESET
#define CLI_ERROR(msg)          "> " CLI_COLOR_RED    msg "\r\n" CLI_COLOR_RESET

#define CLI_COLOR_RED           "\x1B[31m"
#define CLI_COLOR_GREEN         "\x1B[32m"
#define CLI_COLOR_YELLOW        "\x1B[33m"
#define CLI_COLOR_BLUE          "\x1B[34m"
#define CLI_COLOR_CYAN          "\x1B[36m"
#define CLI_COLOR_WHITE         "\x1B[37m"
#define CLI_COLOR_RESET         "\x1B[0m"

#define CLI_ARG_COUNT           (3)
#define CLI_ARG_MAX_SIZE        (64)


extern void cli_init(void);
extern void cli_process(void);


#endif // _CLI_H_
