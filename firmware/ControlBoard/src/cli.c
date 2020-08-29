//  ***************************************************************************
/// @file    cli.c
/// @author  NeoProg
//  ***************************************************************************
#include "cli.h"
#include "project_base.h"
#include "usart1.h"
#include "system_monitor.h"
#include "communication.h"
#include "configurator.h"
#include "servo_driver.h"
#include "motion_core.h"
#include "indication.h"
#include "version.h"

#define COMMUNICATION_BAUD_RATE                     (115200)


typedef enum {
    STATE_NO_INIT,
    STATE_WAIT_FRAME,
    STATE_FRAME_RECEIVED,
    STATE_TRANSMIT
} state_t;


static state_t state = STATE_NO_INIT;


static void frame_received_callback(uint32_t frame_size);
static void frame_transmitted_or_error_callback(void);
static bool parse_command_line(char* cmd_line, char* module, char* cmd, char (*argv)[CLI_ARG_MAX_SIZE], uint8_t* argc);
static bool process_command(const char* module, const char* cmd, char (*argv)[CLI_ARG_MAX_SIZE], uint8_t argc, char* response);


//  ***************************************************************************
/// @brief  CLI driver initialization
/// @param  none
/// @return none
//  ***************************************************************************
void cli_init(void) {
    
    usart1_callbacks_t callbacks;
    callbacks.frame_received_callback = frame_received_callback;
    callbacks.frame_transmitted_callback = frame_transmitted_or_error_callback;
    callbacks.frame_error_callback = frame_transmitted_or_error_callback;
    usart1_init(COMMUNICATION_BAUD_RATE, &callbacks);
    
    state = STATE_WAIT_FRAME;
    usart1_start_rx();
}

//  ***************************************************************************
/// @brief  CLI driver process
/// @note   Call from Main Loop
/// @return none
//  ***************************************************************************
void cli_process(void) {
    
    if (state == STATE_FRAME_RECEIVED) {
        char module[CLI_ARG_MAX_SIZE] = {0};
        char cmd[CLI_ARG_MAX_SIZE] = {0};
        char argv[CLI_ARG_COUNT][CLI_ARG_MAX_SIZE] = {0};
        uint8_t argc = 0;
        
        // Process received command
        char* tx_buffer = (char*)usart1_get_tx_buffer();
        char* rx_buffer = (char*)usart1_get_rx_buffer();
        tx_buffer[0] = '\0';
        if (parse_command_line(rx_buffer, module, cmd, argv, &argc) == true) {
            if (process_command(module, cmd, argv, argc, tx_buffer) == false) {
                if (strlen(tx_buffer) == 0) {
                    strcpy(tx_buffer, CLI_ERROR("ERROR"));
                }
            }
            else {
                if (strlen(tx_buffer) == 0) {
                    strcpy(tx_buffer, CLI_OK("OK"));
                }
            }
        }
        else {
            strcpy(tx_buffer, CLI_ERROR("ERROR"));
        }

        // Send response
        usart1_start_tx(strlen(tx_buffer));
        state = STATE_TRANSMIT;
    }
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
        sprintf(response, CLI_HELP("+------------------------------------------------------------------------+")
                          CLI_HELP("| Artificial intelligence walking machine - CLI help subsystem           |")
                          CLI_HELP("+------------------------------------------------------------------------+")
                          CLI_HELP("")
                          CLI_HELP("Hello. I think you don't understand how work with me?")
                          CLI_HELP("Don't worry! I can help you")
                          CLI_HELP("You can send me command in next format [module] [cmd] [arg 1] ... [arg N]")
                          CLI_HELP("Also you can use all commands from list:")
                          CLI_HELP("")
                          CLI_HELP("basic commands description")
                          CLI_HELP("    - help                                - display this message again")
                          CLI_HELP("    - ?                                   - display this message again")
                          CLI_HELP("\"system\" commands description")
                          CLI_HELP("    - version                             - print firmware version")
                          CLI_HELP("    - status                              - get current system status")
                          CLI_HELP("    - reset                               - reset MCU")
                          CLI_HELP("")
                          CLI_HELP("\"servo\" driver commands description")
                          CLI_HELP("    - calibration <pulse_width>           - start servo calibration")
                          CLI_HELP("    - set_override_level <servo> <level>  - set override level")
                          CLI_HELP("    - set_override_value <servo> <value>  - set override value")
                          CLI_HELP("")
                          CLI_HELP("\"config\" module commands description")
                          CLI_HELP("    - read <page>                         - read page (256 bytes)")
                          CLI_HELP("    - read16 <address> <s|u>              - read 16-bit DEC value")
                          CLI_HELP("    - read32 <address> <s|u>              - read 32-bit DEC value")
                          CLI_HELP("    - write <address> <HEX data>          - write HEX data")
                          CLI_HELP("    - write16 <address> <DEC value>       - write 16-bit DEC value")
                          CLI_HELP("    - write32 <address> <DEC value>       - write 32-bit DEC value")
                          CLI_HELP("    - erase                               - mass erase storage")
                          CLI_HELP("    - calc_checksum <page>                - calculate page checksum")
                          CLI_HELP("    - verify <page>                       - verify page checksum")
                          CLI_HELP("")
                          CLI_HELP("\"indication\" driver commands description")
                          CLI_HELP("    - external-control <0|1>              - enable indication control")
                          CLI_HELP("    - set-state RGBBuzzer              - set state for LEDs and Buzzer")
                          CLI_HELP("")
                          CLI_HELP("For example you can send me next command: system status")
                          CLI_HELP("I hope now you can work with me :)"));
    }
    else if (strcmp(module, "system") == 0) {

        if (strcmp(cmd, "version") == 0) {
            sprintf(response, CLI_OK("Firmware version: %s"), FIRMWARE_VERSION);
        }
        else if (strcmp(cmd, "status") == 0) {
            sprintf(response, CLI_OK("system status report")
                              CLI_OK("    - system_status: 0x%04X")
                              CLI_OK("    - module_status: 0x%04X")
                              CLI_OK("    - battery voltage: %d mV"),
                    sysmon_system_status, sysmon_module_status, sysmon_battery_voltage);
        }
        else if (strcmp(cmd, "reset") == 0) {
            servo_driver_safe_shutdown();
            NVIC_SystemReset();
        }
        else {
            strcpy(response, CLI_ERROR("Unknown command for system"));
            return false;
        }
    }
    else if (strcmp(module, "servo") == 0) {
        return servo_driver_cli_command_process(cmd, argv, argc, response);
    }
    /*else if (strcmp(module, "motion") == 0) {
        return motion_core_cli_command_process(cmd, argv, argc, response);
    }*/
    else if (strcmp(module, "config") == 0) {
        return config_cli_command_process(cmd, argv, argc, response);
    }
    else if (strcmp(module, "indication") == 0) {
        return indication_cli_command_process(cmd, argv, argc, response);
    }
    else {
        strcpy(response, CLI_ERROR("Unknown module name"));
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
    char* word = strtok(cmd_line, " \r\n\0");
    if (word == NULL) {
        return false;
    }
    strcpy(module, word);

    // Parse command
    word = strtok(NULL, " \r\n\0");
    if (word == NULL) {
        return true;
    }
    strcpy(cmd, word);

    // Parse arguments
    for (uint32_t i = 0; i < CLI_ARG_COUNT; ++i) {

        char* word = strtok(NULL, " \r\n\0");
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

//  ***************************************************************************
/// @brief  Frame received callback
/// @param  frame_size: received frame size
/// @return none
//  ***************************************************************************
static void frame_received_callback(uint32_t frame_size) {
    state = STATE_FRAME_RECEIVED;
}

//  ***************************************************************************
/// @brief  Frame transmitter or error callback
/// @param  none
/// @return none
//  ***************************************************************************
static void frame_transmitted_or_error_callback(void) {
    state = STATE_WAIT_FRAME;
    usart1_start_rx();
}
