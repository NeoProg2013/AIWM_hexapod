//  ***************************************************************************
/// @file    cli.c
/// @author  NeoProg
//  ***************************************************************************
#include "cli.h"
#include "stm32f373xc.h"
#include "system_monitor.h"
#include "communication.h"
#include "configurator.h"
#include "servo_driver.h"
#include "version.h"
#include <string.h>
#include <stdio.h>

#include "movement_engine.h"


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

    char module[CLI_ARG_MAX_SIZE] = {0};
    char cmd[CLI_ARG_MAX_SIZE] = {0};
    char argv[CLI_ARG_COUNT][CLI_ARG_MAX_SIZE] = {0};
    uint8_t argc = 0;

    // Parse command line
    if (parse_command_line(rx_buffer, module, cmd, argv, &argc) == true) {

        // Process command
        if (process_command(module, cmd, argv, argc, tx_buffer) == false) {
            if (strlen(tx_buffer) == 0) {
                strcpy(tx_buffer, CLI_MSG("ERROR"));
            }
        }
        else {
            if (strlen(tx_buffer) == 0) {
                strcpy(tx_buffer, CLI_MSG("OK"));
            }
        }
    }
    else {
        strcpy(tx_buffer, CLI_MSG("ERROR"));
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
        sprintf(response, CLI_MSG("+------------------------------------------------------------------------+")
                          CLI_MSG("| Artificial intelligence walking machine - CLI help subsystem           |")
                          CLI_MSG("+------------------------------------------------------------------------+")
                          CLI_MSG("")
                          CLI_MSG("Hello. I think you don't understand how work with me?")
                          CLI_MSG("Don't worry! I can help you")
                          CLI_MSG("You can send me command in next format [module] [cmd] [arg 1] ... [arg N]")
                          CLI_MSG("Also you can use all commands from list:")
                          CLI_MSG("")
                          CLI_MSG("basic commands description")
                          CLI_MSG("    - exit                                - switch to SWLP protocol")
                          CLI_MSG("    - help                                - display this message again")
                          CLI_MSG("    - ?                                   - display this message again")
						  CLI_MSG("\"system\" commands description")
                          CLI_MSG("    - version                             - print firmware version")
                          CLI_MSG("    - status                              - get current system status")
                          CLI_MSG("    - reset                               - reset MCU")
                          CLI_MSG("")
                          CLI_MSG("\"servo\" driver commands description")
						  CLI_MSG("    - calibration <pulse_width>           - start servo calibration")
                          CLI_MSG("    - set_override_level <servo> <level>  - set override level")
                          CLI_MSG("    - set_override_value <servo> <value>  - set override value")
                          CLI_MSG("")
                          CLI_MSG("\"config\" module commands description")
                          CLI_MSG("    - read <page>                         - read page (256 bytes)")
                          CLI_MSG("    - read16 <address> <s|u>              - read 16-bit DEC value")
                          CLI_MSG("    - read32 <address> <s|u>              - read 32-bit DEC value")
                          CLI_MSG("    - write <address> <HEX data>          - write HEX data")
                          CLI_MSG("    - write16 <address> <DEC value>       - write 16-bit DEC value")
                          CLI_MSG("    - write32 <address> <DEC value>       - write 32-bit DEC value")
                          CLI_MSG("    - erase                               - mass erase storage")
                          CLI_MSG("    - calc_checksum <page>                - calculate page checksum")
                          CLI_MSG("    - verify <page>                       - verify page checksum")
                          CLI_MSG("")
                          CLI_MSG("For example you can send me next command: system status")
						  CLI_MSG("I hope now you can work with me :)"));
    }
    else if (strcmp(module, "exit") == 0) {
        communication_switch_to_swlp();
        strcpy(response, CLI_MSG("Main communication protocol - SWLP"));
    }
    else if (strcmp(module, "system") == 0) {

        if (strcmp(cmd, "version") == 0) {
            sprintf(response, CLI_MSG("Firmware version: %s"), FIRMWARE_VERSION);
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
            servo_driver_safe_shutdown();
            NVIC_SystemReset();
        }
        else {
            return false;
        }
    }
    else if (strcmp(module, "servo") == 0) {
        return servo_driver_cli_command_process(cmd, argv, argc, response);
    }
    else if (strcmp(module, "config") == 0) {
        return config_cli_command_process(cmd, argv, argc, response);
    }
    
    else if (strcmp(module, "UP") == 0) {
        movement_engine_select_sequence(SEQUENCE_UP);
    }
    else if (strcmp(module, "DIRECT") == 0) {
        movement_engine_select_sequence(SEQUENCE_DIRECT_MOVEMENT);
    }
    else if (strcmp(module, "NONE") == 0) {
        movement_engine_select_sequence(SEQUENCE_NONE);
    }
    
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
