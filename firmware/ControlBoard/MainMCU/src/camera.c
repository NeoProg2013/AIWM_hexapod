//  ***************************************************************************
/// @file    camera.c
/// @author  NeoProg
//  ***************************************************************************
#include "camera.h"
#include "project_base.h"
#include "usart3.h"
#include "system_monitor.h"
#include "systimer.h"

#define COMMUNICATION_BAUD_RATE             (9600)
#define COMMUNICATION_TIMEOUT               (5000) // ms


typedef enum {
    STATE_NO_INIT,
    STATE_WAIT_FRAME,
    STATE_FRAME_RECEIVED
} state_t;


static state_t state = STATE_NO_INIT;


static void frame_received_callback(uint32_t frame_size);
static void frame_error_callback(void);



//  ***************************************************************************
/// @brief  Camera initialization
//  ***************************************************************************
void camera_init(void) {
    usart3_callbacks_t callbacks;
    callbacks.frame_received_callback = frame_received_callback;
    callbacks.frame_error_callback = frame_error_callback;
    usart3_init(COMMUNICATION_BAUD_RATE, &callbacks);
    
    usart3_start_rx();
    state = STATE_WAIT_FRAME;
}

//  ***************************************************************************
/// @brief  Camera process
//  ***************************************************************************
void camera_process(void) {
    if (sysmon_is_module_disable(SYSMON_MODULE_CAMERA) == true) return; // Module disabled
   
    
    static uint64_t frame_receive_time = (uint64_t)(-1);
    if (frame_receive_time == (uint64_t)(-1)) {
        frame_receive_time = get_time_ms(); // Initialize variable for first function call
    }
    
    if (state == STATE_FRAME_RECEIVED) {
        // Check camera error
        const char* rx_buffer = (const char*)usart3_get_rx_buffer();
        if (strcmp(rx_buffer, "ERROR") == 0) {
            sysmon_disable_module(SYSMON_MODULE_CAMERA);
            return;
        }
        
        // Start RX new frame
        usart3_start_rx();
        state = STATE_WAIT_FRAME;
        
        // Update frame receive time
        frame_receive_time = get_time_ms();
    } 
    
    // Check communication timeout
    sysmon_enable_module(SYSMON_MODULE_CAMERA);
    if (get_time_ms() - frame_receive_time > COMMUNICATION_TIMEOUT) {
        sysmon_disable_module(SYSMON_MODULE_CAMERA);
    }
}





//  ***************************************************************************
/// @brief  Frame received callback
/// @param  frame_size: received frame size none
//  ***************************************************************************
static void frame_received_callback(uint32_t frame_size) {
    state = STATE_FRAME_RECEIVED;
}

//  ***************************************************************************
/// @brief  Frame transmitter or error callback
//  ***************************************************************************
static void frame_error_callback(void) {
    state = STATE_WAIT_FRAME;
    usart3_start_rx();
}
