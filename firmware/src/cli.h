//  ***************************************************************************
/// @file    cli.h
/// @author  NeoProg
/// @brief   Command line interface driver
//  ***************************************************************************
#ifndef _CLI_H_
#define _CLI_H_

#include <stdint.h>


#define CLI_BLACK                   "\x1b[30m"
#define CLI_RED                     "\x1b[31m"
#define CLI_GREEN                   "\x1b[32m"
#define CLI_YELLOW                  "\x1b[33m"
#define CLI_BLUE                    "\x1b[34m"
#define CLI_MAGENTA                 "\x1b[35m"
#define CLI_CYAN                    "\x1b[36m"
#define CLI_WHITE                   "\x1b[37m"
#define CLI_RESET                   "\x1b[0m"

#define CLI_UNDERSCORE              "\x1b[4m"
#define CLI_BRIGHT                  "\x1b[1m"

#define CLI_ERROR_MSG(msg)          CLI_RED    "----> " msg CLI_RESET "\r\n"
#define CLI_OK_MSG(msg)             CLI_GREEN  "----> " msg CLI_RESET "\r\n"
#define CLI_HELP_MSG(msg)           CLI_YELLOW "----> " msg CLI_RESET "\r\n"
#define CLI_MSG(msg)                CLI_CYAN   "----> " msg CLI_RESET "\r\n"

#define CLI_ARG_COUNT               (3)
#define CLI_ARG_MAX_SIZE            (64)


extern void cli_init(void);
extern uint32_t cli_process_frame(char* rx_buffer, char* tx_buffer);


#endif // _CLI_H_
