//  ***************************************************************************
/// @file    smcu.c
/// @author  NeoProg
//  ***************************************************************************
#include "smcu.h"
#include "project_base.h"
#include "usart3.h"
#include "cli.h"
#include "system_monitor.h"
#include "systimer.h"
#define COMMUNICATION_BAUD_RATE             (500000)
#define COMMUNICATION_TIMEOUT               (50) // ms
#define SMCU_FRAME_SIZE                     (20)

typedef enum {
    STATE_NO_INIT,
    STATE_WAIT_FRAME,
    STATE_FRAME_RECEIVED
} state_t;


static state_t state = STATE_NO_INIT;
static uint32_t received_frame_size = 0;
static bool is_enable_data_logging = false;


static void frame_received_callback(uint32_t frame_size);
static void frame_error_callback(void);

typedef struct {
    float x;
    float y;
    float z;
} point_3d_t;

typedef struct {
    float    angle;
    uint16_t length;
    int16_t  zero_rotate;
    
    int16_t  prot_min_angle; // Protection min angle, [degree]
    int16_t  prot_max_angle; // Protection max angle, [degree]
} link_t;

typedef struct {
    point_3d_t position;
    link_t coxa;
    link_t femur;
    link_t tibia;
} limb_t;


extern limb_t g_limbs[6];


//  ***************************************************************************
/// @brief  SMCU driver initialization
//  ***************************************************************************
void smcu_init(void) {
    usart3_callbacks_t callbacks;
    callbacks.frame_received_callback = frame_received_callback;
    callbacks.frame_error_callback = frame_error_callback;
    usart3_init(COMMUNICATION_BAUD_RATE, &callbacks);
    
    usart3_start_rx();
    state = STATE_WAIT_FRAME;
}

//  ***************************************************************************
/// @brief  SMCU driver process
//  ***************************************************************************
void smcu_process(void) {
    static uint64_t frame_receive_time = (uint64_t)(-1);
    if (frame_receive_time == (uint64_t)(-1)) {
        frame_receive_time = get_time_ms(); // Initialize variable for first function call
    }
    
    if (state == STATE_FRAME_RECEIVED) {
        do {
            uint8_t* rx_buffer = usart3_get_rx_buffer();
            uint32_t cursor = 0;
            
            // Check frame
            if (received_frame_size != SMCU_FRAME_SIZE) {
                break;
            }
            if (rx_buffer[0] != 0xAA || rx_buffer[SMCU_FRAME_SIZE - 1] != 0xAA) {
                break;
            }
            ++cursor;
            
            // Read HX711 data
            int16_t hx711_data[6] = {0};
            for (uint32_t i = 0; i < 6; ++i) {
                memcpy(&hx711_data[i], &rx_buffer[cursor], sizeof(hx711_data[i]));
                cursor += sizeof(hx711_data[i]);
            }
            
            // Read MPU6050 data
            int16_t mpu6050_data[3] = {0};
            for (uint32_t i = 0; i < 3; ++i) {
                memcpy(&mpu6050_data[i], &rx_buffer[cursor], sizeof(mpu6050_data[i]));
                cursor += sizeof(mpu6050_data[i]);
            }
            
            if (is_enable_data_logging) {
                char buffer[256] = {0};
                sprintf(buffer, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 
                        hx711_data[0], (int16_t)g_limbs[0].position.y, hx711_data[1], (int16_t)g_limbs[1].position.y, hx711_data[2], (int16_t)g_limbs[2].position.y,
                        hx711_data[3], (int16_t)g_limbs[3].position.y, hx711_data[4], (int16_t)g_limbs[4].position.y, hx711_data[5], (int16_t)g_limbs[5].position.y);
                cli_send_data(buffer);
            }
            
            // Update frame receive time
            frame_receive_time = get_time_ms(); 
        } while (false);

        // Start RX new frame
        usart3_start_rx();
        state = STATE_WAIT_FRAME;
    } 
    
    // Check communication timeout
    sysmon_enable_module(SYSMON_MODULE_SMCU);
    if (get_time_ms() - frame_receive_time > COMMUNICATION_TIMEOUT) {
        sysmon_disable_module(SYSMON_MODULE_SMCU);
    }
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
bool smcu_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response) {
    if (strcmp(cmd, "logging") == 0 && argc == 1) {
        if (argv[0][0] == '1') {
            is_enable_data_logging = true;
        } else {
            is_enable_data_logging = false;
        }
    } else {
        strcpy(response, CLI_ERROR("Unknown command or format for servo driver"));
        return false;
    }
    return true;
}





//  ***************************************************************************
/// @brief  Frame received callback
/// @param  frame_size: received frame size none
//  ***************************************************************************
static void frame_received_callback(uint32_t frame_size) {
    received_frame_size = frame_size;
    state = STATE_FRAME_RECEIVED;
}

//  ***************************************************************************
/// @brief  Frame transmitter or error callback
//  ***************************************************************************
static void frame_error_callback(void) {
    state = STATE_WAIT_FRAME;
    usart3_start_rx();
}
