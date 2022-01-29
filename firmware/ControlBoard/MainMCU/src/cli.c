/// ***************************************************************************
/// @file    cli.c
/// @author  NeoProg
/// ***************************************************************************
#include "cli.h"
#include "project-base.h"
#include "usart1.h"
#include "system-monitor.h"
#include "servo-driver.h"
#include "motion-core.h"
#include "indication.h"
#include "smcu.h"
#include "version.h"
#define COMMUNICATION_BAUD_RATE                     (1000000)


typedef enum {
    STATE_NO_INIT,
    STATE_WAIT_FRAME,
    STATE_FRAME_RECEIVED
} state_t;


static state_t state = STATE_NO_INIT;


static void frame_received_callback(uint32_t frame_size);
static void frame_error_callback(void);
static bool parse_command_line(char* cmd_line, char* module, char* cmd, char (*argv)[CLI_ARG_MAX_SIZE], uint8_t* argc);
static bool process_command(const char* module, const char* cmd, char (*argv)[CLI_ARG_MAX_SIZE], uint8_t argc, char* response);


/// ***************************************************************************
/// @brief  CLI driver initialization
/// @param  none
/// @return none
/// ***************************************************************************
void cli_init(void) {
    usart1_callbacks_t callbacks;
    callbacks.frame_received_callback = frame_received_callback;
    callbacks.frame_error_callback = frame_error_callback;
    usart1_init(COMMUNICATION_BAUD_RATE, &callbacks);
    
    state = STATE_WAIT_FRAME;
    usart1_start_rx();
}

/// ***************************************************************************
/// @brief  CLI driver process
/// @note   Call from Main Loop
/// @return none
/// ***************************************************************************
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
                if (tx_buffer[0] == '\0') {
                    strcpy(tx_buffer, CLI_ERROR("ERROR"));
                }
            }
            else {
                if (tx_buffer[0] == '\0') {
                    strcpy(tx_buffer, CLI_OK("OK"));
                }
            }
        }
        else {
            strcpy(tx_buffer, CLI_ERROR("ERROR"));
        }

        // Send response
        usart1_start_sync_tx(strlen(tx_buffer));
        usart1_start_rx();
        state = STATE_WAIT_FRAME;
    }
}

/// ***************************************************************************
/// @brief  Get TX buffer for send data
/// @return TX buffer address
/// ***************************************************************************
void* cli_get_tx_buffer(void) {
    return usart1_get_tx_buffer();
}

/// ***************************************************************************
/// @brief  CLI send data for logging
/// @param  data: data for send
/// ***************************************************************************
void cli_send_data(const char* data) {
    char* tx_buffer = (char*)usart1_get_tx_buffer();
    if (data != NULL) {
        strcpy(tx_buffer, data);
    }
    usart1_start_sync_tx(strlen(tx_buffer));
}





/// ***************************************************************************
/// @brief  Process command
/// @param  module: module name
/// @param  cmd: command
/// @param  argv: arguments list
/// @param  argc: arguments count
/// @param  response: response
/// @retval response
/// @return true - success, false - error
/// ***************************************************************************
static bool process_command(const char* module, const char* cmd, char (*argv)[CLI_ARG_MAX_SIZE], uint8_t argc, char* response) {
    uint32_t cmd_list_size = 0;
    
    if (strcmp(module, "system") == 0) {
        if (strcmp(cmd, "version") == 0) {
            sprintf(response, CLI_OK("Firmware version: %s"), FIRMWARE_VERSION);
            return true;
        }
        else if (strcmp(cmd, "status") == 0) {
            sprintf(response, CLI_OK("system status report")
                              CLI_OK("    - system_status: 0x%04X")
                              CLI_OK("    - module_status: 0x%04X")
                              CLI_OK("    - battery voltage: %d mV"),
                    sysmon_system_status, sysmon_module_status, sysmon_battery_voltage);
            return true;
        }
        else if (strcmp(cmd, "reset") == 0) {
            servo_driver_power_off();
            NVIC_SystemReset();
        }
    } else if (strcmp(module, "servo") == 0) {
        const cli_cmd_t* cmd_list = servo_get_cmd_list(&cmd_list_size);
        for (uint32_t i = 0; i < cmd_list_size; ++i) {
            if (strcmp(cmd, cmd_list[i].cmd) == 0) {
                return cmd_list[i].handler(argv, argc, response);
            }
        }
    } /*else if (strcmp(module, "motion") == 0) {
        const cli_cmd_t* cmd_list = motion_get_cmd_list(&cmd_list_size);
        for (uint32_t i = 0; i < cmd_list_size; ++i) {
            if (strcmp(cmd, cmd_list[i].cmd) == 0) {
                return cmd_list[i].handler(argv, argc, response);
            }
        }
    }*/ else if (strcmp(module, "indication") == 0) {
        const cli_cmd_t* cmd_list = indication_get_cmd_list(&cmd_list_size);
        for (uint32_t i = 0; i < cmd_list_size; ++i) {
            if (strcmp(cmd, cmd_list[i].cmd) == 0) {
                return cmd_list[i].handler(argv, argc, response);
            }
        }
    } else {
        strcpy(response, CLI_ERROR("Unknown module name"));
    }
    strcpy(response, CLI_ERROR("Unknown command or format"));
    return false;
}

/// ***************************************************************************
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
/// ***************************************************************************
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
        if (strlen(word) >= CLI_ARG_MAX_SIZE) {
            return false;
        }

        strcpy(argv[i], word);
        (*argc)++;
    }
    return true;
}

/// ***************************************************************************
/// @brief  Frame received callback
/// @param  frame_size: received frame size
/// @return none
/// ***************************************************************************
static void frame_received_callback(uint32_t frame_size) {
    state = STATE_FRAME_RECEIVED;
}

/// ***************************************************************************
/// @brief  Frame transmitter or error callback
/// @param  none
/// @return none
/// ***************************************************************************
static void frame_error_callback(void) {
    state = STATE_WAIT_FRAME;
    usart1_start_rx();
}
