//  ***************************************************************************
/// @file    cli_core.c
/// @author  NeoProg
//  ***************************************************************************
#include "cli_core.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CLI_MAX_COMMAND_HISTORY_LENGTH			(5)
#define CLI_MAX_COMMAND_LENGTH					(64) 
#define CLI_GREETING_STRING						("\x1B[36mroot@hexapod-AIWM: \x1B[0m")
#define CLI_ESCAPE_SEQUENCES_COUNT				(11)


typedef void(*escape_handler_t)(void);
typedef struct {
    char* sequence;
    escape_handler_t handler;
    bool is_exclude;
} escape_t;

typedef struct {
    char cmd[CLI_MAX_COMMAND_LENGTH];
    int32_t length;
} cmd_info_t;

typedef enum {
    CLI_STATE_DEFAULT,
    CLI_STATE_ESCAPE
} cli_state_t;


static void escape_state_process(char symbol);
static void default_state_process(char symbol);
static void escape_return_handler(void);
static void escape_backspace_handler(void);
static void escape_del_handler(void);
static void escape_up_handler(void);
static void escape_down_handler(void);
static void escape_left_handler(void);
static void escape_right_handler(void);
static void escape_home_handler(void);
static void escape_end_handler(void);


static const char escape_signature[] = {'\x1B', '\x7F', '\x08', '\x0D', '\x0A' };
static escape_t escape_list[CLI_ESCAPE_SEQUENCES_COUNT] = {
    { .sequence = "\x0D",    .handler = escape_return_handler,    .is_exclude = false },
    { .sequence = "\x0A",    .handler = escape_return_handler,    .is_exclude = false },
    { .sequence = "\x7F",    .handler = escape_backspace_handler, .is_exclude = false },
    { .sequence = "\x08",    .handler = escape_backspace_handler, .is_exclude = false },
    { .sequence = "\x1B[3~", .handler = escape_del_handler,       .is_exclude = false },
    { .sequence = "\x1B[A",  .handler = escape_up_handler,        .is_exclude = false },
    { .sequence = "\x1B[B",  .handler = escape_down_handler,      .is_exclude = false },
    { .sequence = "\x1B[D",  .handler = escape_left_handler,      .is_exclude = false },
    { .sequence = "\x1B[C",  .handler = escape_right_handler,     .is_exclude = false },
    { .sequence = "\x1B[4~", .handler = escape_end_handler,       .is_exclude = false },
    { .sequence = "\x1B[1~", .handler = escape_home_handler,      .is_exclude = false }
};

void(*_send_data)(const char* data) = NULL;

static cli_state_t cli_state = CLI_STATE_DEFAULT; // Current CLI driver state
static int32_t cursor_pos = 0;                    // Current cursor position (equal cursor position in terminal)
static cmd_info_t current_cmd = {0};              // Current command information: command text and length

static char incoming_escape[10] = {0};            // Buffer for escape sequences
static int32_t incoming_escape_length = 0;        // Current escape sequence length
static int32_t possible_escape_sequences_count = CLI_ESCAPE_SEQUENCES_COUNT;

static cmd_info_t cmd_history[CLI_MAX_COMMAND_HISTORY_LENGTH] = {0}; // Command history buffer
static int32_t cmd_history_length = 0;                               // Command history buffer length (command count)
static int32_t cmd_history_pos = 0;                                  // Current position in history buffer (using for navigation)


//  ***************************************************************************
/// @brief  CLI core initialization
/// @param  none
/// @return none
//  ***************************************************************************
void cli_core_init(void(*send_data)(const char*)) {
    _send_data = send_data;
    cli_core_reset();
}

//  ***************************************************************************
/// @brief  Reset CLI core
/// @param  none
/// @return none
//  ***************************************************************************
void cli_core_reset(void) {
    cli_state = CLI_STATE_DEFAULT;
    cursor_pos = 0;
    memset(&current_cmd, 0, sizeof(current_cmd));
    
    memset(incoming_escape, 0, sizeof(incoming_escape));
    incoming_escape_length = 0;
    possible_escape_sequences_count = CLI_ESCAPE_SEQUENCES_COUNT;
    for (int32_t i = 0; i < CLI_ESCAPE_SEQUENCES_COUNT; ++i) {
        escape_list[i].is_exclude = false;
    }
    
    memset(cmd_history, 0, sizeof(cmd_history));
    cmd_history_length = 0;
    cmd_history_pos = 0;
}

//  ***************************************************************************
/// @brief  Process received symbol
/// @param  symbol: received symbol
/// @return none
//  ***************************************************************************
void cli_core_symbol_received(char symbol) {

    // Check escape signature
    if (cli_state == CLI_STATE_DEFAULT) { 
        for (int32_t i = 0; i < sizeof(escape_signature); ++i) {
            if (symbol == escape_signature[i]) {
                 cli_state = CLI_STATE_ESCAPE;
                 break;
            }
        }
    }

    switch (cli_state) {
        case CLI_STATE_DEFAULT:
            default_state_process(symbol);
            break;
        case CLI_STATE_ESCAPE:
            escape_state_process(symbol);
            break; 
    }
}





//  ***************************************************************************
/// @brief  Process state for receive escape sequence
/// @param  symbol: received symbol
/// @return none
//  ***************************************************************************
static void escape_state_process(char symbol) {

    if (incoming_escape_length < sizeof(incoming_escape)) {
        incoming_escape[incoming_escape_length++] = symbol;
    
        for (int32_t i = 0; i < CLI_ESCAPE_SEQUENCES_COUNT; ++i) {
            if (escape_list[i].is_exclude == true) {
                continue; // We exclude this escape sequence on previous iterations
            }
    
            int32_t escape_length = strlen(escape_list[i].sequence);
            for (int32_t a = 0; a < incoming_escape_length && a < escape_length; ++a) {
                if (incoming_escape[a] != escape_list[i].sequence[a]) {
                    escape_list[i].is_exclude = true; // Exclude this escape sequence
                    --possible_escape_sequences_count;
                    break;
                }
            }
        }
    
        if (possible_escape_sequences_count == 1) {
            for (int32_t i = 0; i < CLI_ESCAPE_SEQUENCES_COUNT; ++i) {
                if (escape_list[i].is_exclude == false) {
                    if (strlen(escape_list[i].sequence) != incoming_escape_length) {
                        return; // We receive not all symbols - wait next symbol
                    }
                    escape_list[i].handler();
                    break;
                }
            }
        }
        else {
            return; // We have few candidates for escape - wait next symbol
        }
    }

    // Reset escape state
    incoming_escape_length = 0;
    cli_state = CLI_STATE_DEFAULT;
    for (int32_t i = 0; i < CLI_ESCAPE_SEQUENCES_COUNT; ++i) {
        escape_list[i].is_exclude = false;
    }
    possible_escape_sequences_count = CLI_ESCAPE_SEQUENCES_COUNT;
}

//  ***************************************************************************
/// @brief  Process state for receive command
/// @param  symbol: received symbol
/// @return none
//  ***************************************************************************
static void default_state_process(char symbol) {

    if (cursor_pos < current_cmd.length) {
        memmove(&current_cmd.cmd[cursor_pos + 1], &current_cmd.cmd[cursor_pos], current_cmd.length - cursor_pos); // Offset symbols after cursor
        _send_data("\x1B[s"); // Save CLI cursor position
        for (int32_t i = cursor_pos; i < current_cmd.length + 1; ++i) { // Replace old symbols
            _send_data(&current_cmd.cmd[i]);
        }
        _send_data("\x1B[u"); // Save CLI cursor position
    }
    current_cmd.cmd[cursor_pos] = symbol;
    ++current_cmd.length;
    ++cursor_pos;
    
    _send_data(&symbol);
}





//  ***************************************************************************
/// @brief  Process '\r' and '\n' escapes
/// @param  none
/// @return none
//  ***************************************************************************
static void escape_return_handler(void) {

    _send_data("\r\n");
    if (strcmp(current_cmd.cmd, "hello") == 0) {
        _send_data(CLI_GREETING_STRING);
        return;
    }

    if (cmd_history_length >= CLI_MAX_COMMAND_HISTORY_LENGTH) { // History buffer overflow - remove first command and offset array to begin
        memmove(&cmd_history[0], &cmd_history[1], sizeof(cmd_history) - sizeof(cmd_history[0]));
        cmd_history_length = CLI_MAX_COMMAND_HISTORY_LENGTH - 1;
    }

    // Put command to history
    cmd_history[cmd_history_length] = current_cmd;
    ++cmd_history_length;

    // Move history cursor to last command
    cmd_history_pos = cmd_history_length;

    // Clear buffer to new command
    memset(&current_cmd, 0, sizeof(current_cmd));
    cursor_pos = 0;

    _send_data(CLI_GREETING_STRING);
}

//  ***************************************************************************
/// @brief  Process BACKSPACE escape
/// @param  none
/// @return none
//  ***************************************************************************
static void escape_backspace_handler(void) {

    if (cursor_pos > 0) {
        memmove(&current_cmd.cmd[cursor_pos - 1], &current_cmd.cmd[cursor_pos], current_cmd.length - cursor_pos); // Remove symbol from buffer
        current_cmd.cmd[current_cmd.length - 1] = 0; // Clear last symbol
        --current_cmd.length;                    // Decreate command size
        --cursor_pos;                            // Shift cursor to left
        
        _send_data("\x7F\x1B[s"); // Remove symbol and save CLI cursor position
        _send_data(&current_cmd.cmd[cursor_pos]); // Replace old symbols
        _send_data(" \x1B[u");    // Hide last symbol and restore CLI cursor position
    }
}

//  ***************************************************************************
/// @brief  Process DELETE escapes
/// @param  none
/// @return none
//  ***************************************************************************
static void escape_del_handler(void) {

    if (cursor_pos < current_cmd.length) {
        memmove(&current_cmd.cmd[cursor_pos], &current_cmd.cmd[cursor_pos + 1], current_cmd.length - cursor_pos); // Remove symbol from buffer
        current_cmd.cmd[current_cmd.length] = 0; // Clear last symbol
        --current_cmd.length;                    // Decreate command size
        
        _send_data("\x1B[s"); // Save CLI cursor position
        _send_data(&current_cmd.cmd[cursor_pos]);
        _send_data(" \x1B[u"); // Hide last symbol and restore CLI cursor position
    }
}

//  ***************************************************************************
/// @brief  Process ARROW_UP escape
/// @param  none
/// @return none
//  ***************************************************************************
static void escape_up_handler(void) {

    --cmd_history_pos;
    if (cmd_history_pos < 0) {
        cmd_history_pos = 0;
    }

    // Calculate diff between current command and command from history
    int32_t remainder = current_cmd.length - cmd_history[cmd_history_pos].length;

    // Move cursor to begin of command
    while (current_cmd.length) {
        _send_data("\x1B[D");
        --current_cmd.length;
        --cursor_pos;
    }

    // Print new command
    current_cmd = cmd_history[cmd_history_pos];
    _send_data(current_cmd.cmd);
    cursor_pos += current_cmd.length;
        
    // Clear others symbols
    _send_data("\x1B[s"); // Save CLI cursor position
    for (int32_t i = 0; i < remainder; ++i) {
        _send_data(" ");
    }
    _send_data("\x1B[u"); // Restore CLI cursor position
}

//  ***************************************************************************
/// @brief  Process ARROW_DOWN escape
/// @param  none
/// @return none
//  ***************************************************************************
static void escape_down_handler(void) {

    ++cmd_history_pos;
    if (cmd_history_pos > cmd_history_length) {
        cmd_history_pos = cmd_history_length;
    }

    int32_t remainder = 0;
    if (cmd_history_pos < cmd_history_length) {

        // Calculate diff between current command and command from history
        remainder = current_cmd.length - cmd_history[cmd_history_pos].length;
    
        // Move cursor to begin of command
        while (current_cmd.length > 0) {
            _send_data("\x1B[D");
            --current_cmd.length;
            --cursor_pos;
        }
    
        // Print new command
        current_cmd = cmd_history[cmd_history_pos];
        _send_data(current_cmd.cmd);
        cursor_pos += current_cmd.length;
    }
    else {
        remainder = current_cmd.length;

        // Move cursor to begin of command
        while (current_cmd.length > 0) {
            _send_data("\x1B[D");
            --current_cmd.length;
            --cursor_pos;
        }

        memset(&current_cmd, 0, sizeof(current_cmd));
    }

    // Clear others symbols
    _send_data("\x1B[s"); // Save CLI cursor position
    for (int32_t i = 0; i < remainder; ++i) {
        _send_data(" ");
    }
    _send_data("\x1B[u"); // Restore CLI cursor position
}

//  ***************************************************************************
/// @brief  Process ARROW_LEFT escape
/// @param  none
/// @return none
//  ***************************************************************************
static void escape_left_handler(void) {
    if (cursor_pos > 0) {
        _send_data("\x1B[D");
        --cursor_pos;
    }
}

//  ***************************************************************************
/// @brief  Process ARROW_RIGHT escape
/// @param  none
/// @return none
//  ***************************************************************************
static void escape_right_handler(void) {
    if (cursor_pos < current_cmd.length) {
        _send_data("\x1B[C");
        ++cursor_pos;
    }
}

//  ***************************************************************************
/// @brief  Process HOME escape
/// @param  none
/// @return none
//  ***************************************************************************
static void escape_home_handler(void) {
    while (cursor_pos > 0) {
        _send_data("\x1B[D");
        --cursor_pos;
    }
}

//  ***************************************************************************
/// @brief  Process END escape
/// @param  none
/// @return none
//  ***************************************************************************
static void escape_end_handler(void) {
    while (cursor_pos < current_cmd.length) {
        _send_data("\x1B[C");
        ++cursor_pos;
    }
}
