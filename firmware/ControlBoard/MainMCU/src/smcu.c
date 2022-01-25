//  ***************************************************************************
/// @file    smcu.c
/// @author  NeoProg
//  ***************************************************************************
#include "smcu.h"
#include "project-base.h"
#include "usart3.h"
#include "cli.h"
#include "system-monitor.h"
#include "systimer.h"
#define COMMUNICATION_BAUD_RATE             (500000)
#define COMMUNICATION_TIMEOUT               (50) // ms
#define SMCU_FRAME_SIZE                     (22)

typedef enum {
    STATE_NO_INIT,
    STATE_WAIT_FRAME,
    STATE_FRAME_RECEIVED
} state_t;


static state_t  state = STATE_NO_INIT;
static uint32_t received_frame_size = 0;
static int16_t  foot_sensors_data[6] = {0};
static int32_t  accel_sensor_data[2] = {0}; // 0.0001*


static void frame_received_callback(uint32_t frame_size);
static void frame_error_callback(void);


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
            // Check frame
            uint8_t* rx_buffer = usart3_get_rx_buffer();
            if (received_frame_size != SMCU_FRAME_SIZE || rx_buffer[0] != 0xAA || rx_buffer[SMCU_FRAME_SIZE - 1] != 0xAA) {
                break;
            }
            uint32_t cursor = 1;
            
            // Read HX711 data
            for (uint32_t i = 0; i < sizeof(foot_sensors_data) / sizeof(foot_sensors_data[0]); ++i) {
                memcpy(&foot_sensors_data[i], &rx_buffer[cursor], sizeof(foot_sensors_data[i]));
                cursor += sizeof(foot_sensors_data[i]);
            }
            
            // Read MPU6050 data
            for (uint32_t i = 0; i < sizeof(accel_sensor_data) / sizeof(accel_sensor_data[0]); ++i) {
                memcpy(&accel_sensor_data[i], &rx_buffer[cursor], sizeof(accel_sensor_data[i]));
                cursor += sizeof(accel_sensor_data[i]);
            }
            
            // Update frame receive time
            frame_receive_time = get_time_ms(); 
        } 
        while (false);

        // Start RX new frame
        usart3_start_rx();
        state = STATE_WAIT_FRAME;
    } 
    
    // Check communication timeout
    sysmon_enable_module(SYSMON_MODULE_SMCU);
    if (get_time_ms() - frame_receive_time > COMMUNICATION_TIMEOUT) {
        memset(foot_sensors_data, 0, sizeof(foot_sensors_data));
        memset(accel_sensor_data, 0, sizeof(accel_sensor_data));
        sysmon_disable_module(SYSMON_MODULE_SMCU);
    }
}

//  ***************************************************************************
/// @brief  Get sensors data
/// @param  foot_sensors: pointer for get address to foot sensor data
/// @param  accel_sensor: pointer for get address to accel sensor data
//  ***************************************************************************
void smcu_get_sensor_data(int16_t** foot_sensors, int32_t** accel_sensor) {
    *foot_sensors = foot_sensors_data;
    *accel_sensor = accel_sensor_data;
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
