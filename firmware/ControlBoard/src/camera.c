//  ***************************************************************************
/// @file    camera.c
/// @author  NeoProg
//  ***************************************************************************
#include "camera.h"
#include "project_base.h"
#include "usart3.h"
#include "system_monitor.h"
#include "systimer.h"

#define IP_ADDRESS_DUMMY                    ("255.255.255.255")
#define COMMUNICATION_BAUD_RATE             (9600)
#define COMMUNICATION_TIMEOUT               (5000) // ms


typedef enum {
    STATE_NO_INIT,
    STATE_WAIT_FRAME,
    STATE_FRAME_RECEIVED
} state_t;


static state_t state = STATE_NO_INIT;
static uint8_t ip_address[] = IP_ADDRESS_DUMMY;
static uint32_t recv_frame_size = 0;


static bool check_ip_address(const char* ip_address, uint32_t len);
static void frame_received_callback(uint32_t frame_size);
static void frame_error_callback(void);



//  ***************************************************************************
/// @brief  Camera initialization
/// @param  none
/// @return none
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
/// @param  none
/// @return none
//  ***************************************************************************
void camera_process(void) {
    
    static uint64_t frame_receive_time = (uint64_t)(-1);
    if (frame_receive_time == (uint64_t)(-1)) {
        frame_receive_time = get_time_ms(); // Initialize variable for first function call
    }
    
    if (state == STATE_FRAME_RECEIVED) {
        
        const char* rx_buffer = (const char*)usart3_get_rx_buffer();
        
        // Check camera error
        sysmon_enable_module(SYSMON_MODULE_CAMERA);
        if (strcmp(rx_buffer, "ERROR") == 0) {
            memcpy(ip_address, IP_ADDRESS_DUMMY, sizeof(IP_ADDRESS_DUMMY));
            sysmon_disable_module(SYSMON_MODULE_CAMERA);
            return;
        }
        
        // Check received frame size
        if (check_ip_address(rx_buffer, recv_frame_size)) {
            
            // Update IP address
            memset(ip_address, 0, sizeof(ip_address));
            memcpy(ip_address, rx_buffer, recv_frame_size);
            
            // Update frame receive time
            frame_receive_time = get_time_ms();
        }
        
        // Start RX new frame
        usart3_start_rx();
        state = STATE_WAIT_FRAME;
    }
    
    // Check communication 
    sysmon_enable_module(SYSMON_MODULE_CAMERA);
    if (get_time_ms() - frame_receive_time > COMMUNICATION_TIMEOUT) {
        memcpy(ip_address, IP_ADDRESS_DUMMY, sizeof(IP_ADDRESS_DUMMY));
        sysmon_disable_module(SYSMON_MODULE_CAMERA);
    }
}

//  ***************************************************************************
/// @brief  Get camera IP address
/// @param  none
/// @return none
//  ***************************************************************************
void camera_get_ip_address(uint8_t* buffer) {
    memcpy(buffer, ip_address, sizeof(ip_address));
}

//  ***************************************************************************
/// @brief  CLI command process
/// @param  cmd: command string
/// @param  argv: argument list
/// @param  argc: arguments count
/// @param  response: response
/// @retval response
/// @return true - success, false - fail
//  ***************************************************************************
bool camera_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response) {

    if (strcmp(cmd, "ip") == 0) {
        sprintf(response, CLI_OK("IP: %s"), (const char*)ip_address);
    }
    else {
        strcpy(response, CLI_ERROR("Unknown command or format for camera"));
        return false;
    }
    return true;
}





//  ***************************************************************************
/// @brief  Check IP address
/// @param  ip_address: IP address string
/// @param  len: IP address len
/// @return none
//  ***************************************************************************
static bool check_ip_address(const char* ip_address, uint32_t len) {
    
    if (len > sizeof(IP_ADDRESS_DUMMY)) {
        return false;
    }
    
    // Check first byte
    uint32_t digits_count = 0;
    uint32_t points_count = 0;
    for (uint32_t i = 0; i < len; ++i) {
        
        char c = ip_address[i];
        if (c > '0' && c < '9') {
            digits_count++;
        }
        else if (c == '.') {
            if (digits_count == 0 || digits_count > 3) {
                return false;
            }
            points_count++;
            digits_count = 0;
            if (points_count > 3) {
                return false;
            }
        }
    }
    return points_count == 3 && digits_count != 0;
}

//  ***************************************************************************
/// @brief  Frame received callback
/// @param  frame_size: received frame size
/// @return none
//  ***************************************************************************
static void frame_received_callback(uint32_t frame_size) {
    recv_frame_size = frame_size;
    state = STATE_FRAME_RECEIVED;
}

//  ***************************************************************************
/// @brief  Frame transmitter or error callback
/// @param  none
/// @return none
//  ***************************************************************************
static void frame_error_callback(void) {
    state = STATE_WAIT_FRAME;
    usart3_start_rx();
}
