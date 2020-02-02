//  ***************************************************************************
/// @file    cli.c
/// @author  NeoProg
//  ***************************************************************************
#include "cli.h"
//#include "memory_map.h"
//#include "servo_driver.h"
#include "system_monitor.h"
#include "communication.h"
//#include "configurator.h"
#include <string.h>
#include <stdio.h>


static bool parse_command_line(char* cmd_line, char* module, char* cmd, char (*argv)[CLI_ARG_MAX_SIZE], uint8_t* argc);
static bool process_command(const char* module, const char* cmd, char (*argv)[CLI_ARG_MAX_SIZE], uint8_t argc, char* response);


//  ***************************************************************************
/// @brief  CLI driver initialization
/// @param  none
/// @return none
//  ***************************************************************************
void cli_init(void) {

}

//  ***************************************************************************
/// @brief  Process received frame
/// @param  rx_buffer: RX data
/// @param  tx_buffer: buffer for TX data
/// @return none
//  ***************************************************************************
uint32_t cli_process_frame(char* rx_buffer, char* tx_buffer) {

    char cli_module[CLI_ARG_MAX_SIZE] = {0};
    char cli_cmd[CLI_ARG_MAX_SIZE] = {0};
    char cli_argv[CLI_ARG_COUNT][CLI_ARG_MAX_SIZE] = {0};
    uint8_t cli_argc = 0;

    // Parse command line
    if (parse_command_line(rx_buffer, cli_module, cli_cmd, cli_argv, &cli_argc) == true) {

        // Process command
        if (process_command(cli_module, cli_cmd, cli_argv, cli_argc, tx_buffer) == false) {
            if (strlen(tx_buffer) == 0) {
                strcpy(tx_buffer, CLI_ERROR_MSG("ERROR"));
            }
        }
        else {
            if (strlen(tx_buffer) == 0) {
                strcpy(tx_buffer, CLI_OK_MSG("OK"));
            }
        }
    }
    else {
        strcpy(tx_buffer, CLI_ERROR_MSG("ERROR"));
    }

    // Return response size
    return strlen(tx_buffer);
}





//  ***************************************************************************
/// @brief  Process command
/// @param  module: module name
/// @param  cmd: command
/// @param  argv: arguments list
/// @param  argc: arguments count
/// @param  response: response
/// @retval response
/// @return true - success, false - error
//  ***************************************************************************
static bool process_command(const char* module, const char* cmd, char (*argv)[CLI_ARG_MAX_SIZE], uint8_t argc, char* response) {

    if (strcmp(module, "help") == 0 || strcmp(module, "?") == 0) {
        sprintf(response, CLI_HELP_MSG("+------------------------------------------------------------------------+")
                          CLI_HELP_MSG("| Artificial intelligence walking machine - CLI help subsystem           |")
                          CLI_HELP_MSG("+------------------------------------------------------------------------+")
                          CLI_HELP_MSG("")
                          CLI_HELP_MSG("Hello. I think you don't understand how work with?")
                          CLI_HELP_MSG("Don't worry! I can help you")
                          CLI_HELP_MSG("You can send me command in next format [module] [cmd] [arg 1] ... [arg N]")
                          CLI_HELP_MSG("Also you can use all commands from list:")
                          CLI_HELP_MSG("")
                          CLI_HELP_MSG(CLI_UNDERSCORE CLI_BRIGHT "basic commands description")
                          CLI_HELP_MSG("    - swp                                 - switch to SWP protocol")
                          CLI_HELP_MSG("    - help                                - display this message again")
                          CLI_HELP_MSG("    - ?                                   - display this message again")
						  CLI_HELP_MSG(CLI_UNDERSCORE CLI_BRIGHT "\"system\" commands description")
                          CLI_HELP_MSG("    - version                             - print firmware version")
                          CLI_HELP_MSG("    - status                              - get current system status")
                          CLI_HELP_MSG("    - reset                               - reset MCU")
                          CLI_HELP_MSG("")
                          CLI_HELP_MSG(CLI_UNDERSCORE CLI_BRIGHT "\"servo\" driver commands description")
						  CLI_HELP_MSG("    - attach <servo>                      - attach servo to driver")
                          CLI_HELP_MSG("    - detach <servo>                      - detach servo from driver")
                          CLI_HELP_MSG("    - status <servo>                      - get current servo status")
						  CLI_HELP_MSG("    - set_angle <servo> <log|phy> <angle> - set servo angle")
                          CLI_HELP_MSG("    - set_pulse <servo> <pulse>           - set servo pulse width")
                          CLI_HELP_MSG("")
                          CLI_HELP_MSG(CLI_UNDERSCORE CLI_BRIGHT "\"config\" module commands description")
                          CLI_HELP_MSG("    - read <page>                         - read page")
                          CLI_HELP_MSG("    - write <address> <bytes> <data>      - write data")
                          CLI_HELP_MSG("    - erase                               - erase all configurations")
                          CLI_HELP_MSG("    - calc_checksum                       - calculate and write checksum")
                          CLI_HELP_MSG("")
                          CLI_HELP_MSG("For example you can send me next command: system status")
						  CLI_HELP_MSG("I hope now you can work with me :)"));
    }
    else if (strcmp(module, "swp") == 0) {
        communication_switch_to_swp();
        strcpy(response, CLI_MSG("Main communication protocol - SWP"));
    }
    else if (strcmp(module, "system") == 0) {

        if (strcmp(cmd, "version") == 0) {
            sprintf(response, CLI_MSG("Firmware version: %d.%d.%d"), 1, 0, 255);
        }
        else if (strcmp(cmd, "status") == 0) {
            sprintf(response, CLI_MSG("system status report")
                              CLI_MSG("    - system_status: 0x%04X")
                              CLI_MSG("    - module_status: 0x%04X")
                              CLI_MSG("    - battery voltage: %d mV")
                              CLI_MSG("    - battery cell #1: %d mV")
                              CLI_MSG("    - battery cell #2: %d mV")
                              CLI_MSG("    - battery cell #3: %d mV"),
                    sysmon_system_status, sysmon_module_status,
                    sysmon_battery_voltage,
                    sysmon_battery_cell_voltage[0],
                    sysmon_battery_cell_voltage[1],
                    sysmon_battery_cell_voltage[2]);
        }
        else if (strcmp(cmd, "reset") == 0) {

            // Reset MCU
        }
        else {
            return false;
        }
    }
    /*else if (strcmp(module, "servo") == 0) {
        return servo_driver_cli_command_process(cmd, argv, argc, response);
    }
    else if (strcmp(module, "config") == 0) {
        return config_cli_command_process(cmd, argv, argc, response);
    }*/
    else {
        return false;
    }
    return true;
}

//  ***************************************************************************
/// @brief  Parse command line
/// @param  cmd_line: command line
/// @param  module: module name
/// @param  cmd: command
/// @param  argv: arguments list
/// @param  argc: arguments count
/// @retval module
/// @retval cmd
/// @retval argv
/// @retval argc
/// @return none
//  ***************************************************************************
static bool parse_command_line(char* cmd_line, char* module, char* cmd, char (*argv)[CLI_ARG_MAX_SIZE], uint8_t* argc) {

    // Parse module name
    char* word = strtok(cmd_line, " \r\n");
    if (word == NULL) {
        return false;
    }
    strcpy(module, word);

    // Parse command
    word = strtok(NULL, " \r\n");
    if (word == NULL) {
        return true;
    }
    strcpy(cmd, word);

    // Parse arguments
    for (uint32_t i = 0; i < CLI_ARG_COUNT; ++i) {

        char* word = strtok(NULL, " \r\n");
        if (word == NULL) {
            break;
        }
        if (strlen(word) > CLI_ARG_MAX_SIZE) {
            return false;
        }

        strcpy(argv[i], word);
        (*argc)++;
    }
    return true;
}
