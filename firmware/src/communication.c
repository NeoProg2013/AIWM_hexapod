//  ***************************************************************************
/// @file    communication.c
/// @author  NeoProg
//  ***************************************************************************
#include "communication.h"
#include "cli.h"
#include "swlp.h"
#include "usart2.h"
#include "system_monitor.h"
#include "systimer.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define COMMUNICATION_BAUD_RATE                     (115200)
#define COMMUNICATION_TIMEOUT                       (1000)


typedef enum {
    STATE_NO_INIT,
    STATE_WAIT_FRAME,
    STATE_FRAME_RECEIVED,
    STATE_TRANSMIT
} driver_state_t;


static driver_state_t driver_state = STATE_NO_INIT;

static uint8_t rx_buffer[512] = {0};
static uint8_t tx_buffer[3072] = {0};
static uint32_t received_frame_size = 0;
static bool is_switched_to_cli = false;


static void frame_received_callback(uint32_t frame_size);
static void frame_transmitted_or_error_callback(void);


//  ***************************************************************************
/// @brief  Communication driver initialization
/// @param  none
/// @return none
//  ***************************************************************************
void communication_init(void) {
    
    cli_init();
    swlp_init();

    usart2_callbacks_t callbacks;
    callbacks.frame_received_callback = frame_received_callback;
    callbacks.frame_transmitted_callback = frame_transmitted_or_error_callback;
    callbacks.error_callback = frame_transmitted_or_error_callback;
    usart2_init(COMMUNICATION_BAUD_RATE, &callbacks);
    
    is_switched_to_cli = false;
    
    driver_state = STATE_WAIT_FRAME;
    usart2_start_rx(rx_buffer, sizeof(rx_buffer));
}

//  ***************************************************************************
/// @brief  Communication driver process
/// @param  none
/// @return none
//  ***************************************************************************
void communication_process(void) {
    
    // We start with SYSMON_CONN_LOST_ERROR error
    static uint64_t frame_received_time = 0;
    static bool is_first_frame_received = false;

    
    if (driver_state == STATE_FRAME_RECEIVED) {
        
        uint32_t bytes_for_tx = 0;

        // Check switch CLI sequence
		memset(tx_buffer, 0, sizeof(tx_buffer));
        if (rx_buffer[0] == 'c' && rx_buffer[1] == 'l' && rx_buffer[2] == 'i') {
            is_switched_to_cli = true;
            strcpy((char*)tx_buffer, CLI_MSG("Main communication protocol - CLI"));
            bytes_for_tx = strlen((char*)tx_buffer);
        }
        else { // Process communication protocol

            if (is_switched_to_cli == true) {
                bytes_for_tx = cli_process_frame((char*)rx_buffer, (char*)tx_buffer);
            }
            else {
                bytes_for_tx = swlp_process_frame(rx_buffer, received_frame_size, tx_buffer);
            }
        }
		memset(rx_buffer, 0, sizeof(rx_buffer));

        
        if (bytes_for_tx != 0) {
            // Process TX data
            driver_state = STATE_TRANSMIT;
            usart2_start_tx(tx_buffer, bytes_for_tx);
        }
        else {
            // Start RX new frame
            driver_state = STATE_WAIT_FRAME;
            usart2_start_rx(rx_buffer, sizeof(rx_buffer)); 
        }
        
        // Update frame receive time
        frame_received_time = get_time_ms();
        is_first_frame_received = true;
    }
    
    
    //
    // Process communication timeout feature
    //
    sysmon_clear_error(SYSMON_CONN_LOST_ERROR);
    if ((get_time_ms() - frame_received_time > COMMUNICATION_TIMEOUT && is_switched_to_cli == false) || is_first_frame_received == false) {
        sysmon_set_error(SYSMON_CONN_LOST_ERROR);
    }
}

//  ***************************************************************************
/// @brief  Communication driver switch to SWP protocol
/// @param  none
/// @return none
//  ***************************************************************************
void communication_switch_to_swlp(void) {
    is_switched_to_cli = false;
}

//  ***************************************************************************
/// @brief  Frame received callback
/// @param  frame_size: received frame size
/// @return none
//  ***************************************************************************
static void frame_received_callback(uint32_t frame_size) {
    driver_state = STATE_FRAME_RECEIVED;
    received_frame_size = frame_size;
}

//  ***************************************************************************
/// @brief  Frame transmitter or error callback
/// @param  none
/// @return none
//  ***************************************************************************
static void frame_transmitted_or_error_callback(void) {
    usart2_start_rx(rx_buffer, sizeof(rx_buffer));
    driver_state = STATE_WAIT_FRAME;
}
