//  ***************************************************************************
/// @file    swlp.c
/// @author  NeoProg
//  ***************************************************************************
#include "swlp.h"
#include "project_base.h"
#include "swlp_protocol.h"
#include "usart2.h"
#include "sequences_engine.h"
#include "indication.h"
#include "camera.h"
#include "system_monitor.h"
#include "servo_driver.h"
#include "systimer.h"

#define COMMUNICATION_BAUD_RATE                     (115200)
#define COMMUNICATION_TIMEOUT                       (1000)


typedef enum {
    STATE_NO_INIT,
    STATE_WAIT_FRAME,
    STATE_FRAME_RECEIVED,
    STATE_TRANSMIT
} state_t;


static state_t state = STATE_NO_INIT;
static uint32_t received_frame_size = 0;


static void frame_received_callback(uint32_t frame_size);
static void frame_transmitted_or_error_callback(void);
static bool check_frame(const uint8_t* rx_buffer, uint32_t frame_size);
static uint16_t calculate_crc16(const uint8_t* frame, uint32_t size);


//  ***************************************************************************
/// @brief  SWLP driver initialization
/// @param  none
/// @return none
//  ***************************************************************************
void swlp_init(void) {
    usart2_callbacks_t callbacks;
    callbacks.frame_received_callback = frame_received_callback;
    callbacks.frame_transmitted_callback = frame_transmitted_or_error_callback;
    callbacks.frame_error_callback = frame_transmitted_or_error_callback;
    usart2_init(COMMUNICATION_BAUD_RATE, &callbacks);
    
    sysmon_set_error(SYSMON_CONN_LOST_ERROR);
    
    state = STATE_WAIT_FRAME;
    usart2_start_rx();
}

//  ***************************************************************************
/// @brief  Process received frame
/// @param  rx_buffer: received data
/// @param  frame_size: frame size
/// @param  tx_buffer: transmit data
/// @retval tx_buffer
/// @return bytes for transmit from tx_buffer
//  ***************************************************************************
void swlp_process(void) {
    
    // We are start with SYSMON_CONN_LOST_ERROR error
    static uint64_t frame_receive_time = 0;
    if (state == STATE_FRAME_RECEIVED) {
        
        uint8_t* tx_buffer = usart2_get_tx_buffer();
        uint8_t* rx_buffer = usart2_get_rx_buffer();

        // Check frame
        if (check_frame(rx_buffer, received_frame_size) == false) {
            state = STATE_WAIT_FRAME;
            usart2_start_rx();
            return;
        }

        // Preparing
        const swlp_frame_t* swlp_rx_frame = (const swlp_frame_t*)rx_buffer;
        const swlp_command_payload_t* request = (const swlp_command_payload_t*)swlp_rx_frame->payload;

        swlp_frame_t* swlp_tx_frame = (swlp_frame_t*)tx_buffer;
        swlp_status_payload_t* response = (swlp_status_payload_t*)swlp_tx_frame->payload;
        memset(swlp_tx_frame, 0, sizeof(swlp_frame_t));

        // Process command
        response->command_status = SWLP_CMD_STATUS_OK;
        switch (request->command) {
            case SWLP_CMD_NONE:
                break;
            case SWLP_CMD_SELECT_SEQUENCE_UP:
                servo_driver_set_speed(SERVO_DEFAULT_SPEED);
                sequences_engine_select_sequence(SEQUENCE_UP, 0, 0);
                break;
            case SWLP_CMD_SELECT_SEQUENCE_DOWN:
                servo_driver_set_speed(SERVO_DEFAULT_SPEED);
                sequences_engine_select_sequence(SEQUENCE_DOWN, 0, 0);
                break;
            case SWLP_CMD_SELECT_SEQUENCE_DIRECT:
                servo_driver_set_speed(request->motion_speed);
                sequences_engine_select_sequence(SEQUENCE_DIRECT, request->curvature, request->distance);
                break;
            case SWLP_CMD_SELECT_SEQUENCE_REVERSE:
                servo_driver_set_speed(request->motion_speed);
                sequences_engine_select_sequence(SEQUENCE_REVERSE, request->curvature, request->distance);
                break;
            case SWLP_CMD_SELECT_SEQUENCE_UP_DOWN:
                servo_driver_set_speed(request->motion_speed);
                sequences_engine_select_sequence(SEQUENCE_UP_DOWN, 0, 0);
                break;
            case SWLP_CMD_SELECT_SEQUENCE_PUSH_PULL:
                servo_driver_set_speed(request->motion_speed);
                sequences_engine_select_sequence(SEQUENCE_PUSH_PULL, 0, 0);
                break;
            case SWLP_CMD_SELECT_SEQUENCE_ATTACK_LEFT:
                servo_driver_set_speed(SERVO_DEFAULT_SPEED);
                sequences_engine_select_sequence(SEQUENCE_ATTACK_LEFT, 0, 0);
                break;
            case SWLP_CMD_SELECT_SEQUENCE_ATTACK_RIGHT:
                servo_driver_set_speed(SERVO_DEFAULT_SPEED);
                sequences_engine_select_sequence(SEQUENCE_ATTACK_RIGHT, 0, 0);
                break;
            case SWLP_CMD_SELECT_SEQUENCE_DANCE:
                servo_driver_set_speed(SERVO_DEFAULT_SPEED);
                sequences_engine_select_sequence(SEQUENCE_DANCE, 0, 0);
                break;
            case SWLP_CMD_SELECT_SEQUENCE_ROTATE_X:
                servo_driver_set_speed(SERVO_DEFAULT_SPEED);
                sequences_engine_select_sequence(SEQUENCE_ROTATE_X, 0, 0);
                break;
            case SWLP_CMD_SELECT_SEQUENCE_ROTATE_Z:
                servo_driver_set_speed(SERVO_DEFAULT_SPEED);
                sequences_engine_select_sequence(SEQUENCE_ROTATE_Z, 0, 0);
                break;
            case SWLP_CMD_SELECT_SEQUENCE_NONE:
                servo_driver_set_speed(request->motion_speed);
                sequences_engine_select_sequence(SEQUENCE_NONE, 0, 0);
                break;
                
            default:
                response->command_status = SWLP_CMD_STATUS_ERROR;
        }

        // Prepare status payload
        response->command = request->command;
        response->module_status = sysmon_module_status;
        response->system_status = sysmon_system_status;
        response->battery_voltage = sysmon_battery_voltage;
        response->battery_charge = sysmon_battery_charge;
        camera_get_ip_address(response->camera_ip);

        // Prepare response
        swlp_tx_frame->start_mark = SWLP_START_MARK_VALUE;
        swlp_tx_frame->crc16 = calculate_crc16((uint8_t*)swlp_tx_frame, sizeof(swlp_frame_t) - 2);
        
        // Transmit response
        state = STATE_TRANSMIT;
        usart2_start_tx(sizeof(swlp_frame_t));
        
        // Update frame receive time
        frame_receive_time = get_time_ms();
    }
    
    //
    // Process communication timeout feature
    //
    sysmon_clear_error(SYSMON_CONN_LOST_ERROR);
    if (get_time_ms() - frame_receive_time > COMMUNICATION_TIMEOUT || frame_receive_time == 0) {
        sysmon_set_error(SYSMON_CONN_LOST_ERROR);
    }
}





//  ***************************************************************************
/// @brief  Check SWP frame
/// @param  rx_buffer: frame
/// @param  frame_size: frame size
/// @return true - frame valid, false - frame invalid
//  ***************************************************************************
static bool check_frame(const uint8_t* rx_buffer, uint32_t frame_size) {

    // Check frame size
    if (frame_size != sizeof(swlp_frame_t)) {
        return false;
    }

    // Check frame CRC16
    uint16_t crc = calculate_crc16(rx_buffer, frame_size - 2);
    if (calculate_crc16(rx_buffer, frame_size) != 0) {
        return false;
    }

    // Check start mark
    const swlp_frame_t* swlp_frame = (const swlp_frame_t*)rx_buffer;
    if (swlp_frame->start_mark != SWLP_START_MARK_VALUE) {
        return false;
    }

    return true;
}

//  ***************************************************************************
/// @brief  Calculate frame CRC16
/// @param  frame: frame
/// @param  size: frame size
/// @return CRC16 value
//  ***************************************************************************
static uint16_t calculate_crc16(const uint8_t* frame, uint32_t size) {

    uint16_t crc16 = 0xFFFF;
    uint16_t data = 0;
    uint16_t k = 0;

    while (size--) {
        crc16 ^= *frame++;
        k = 8;
        while (k--) {
            data = crc16;
            crc16 >>= 1;
            if (data & 0x0001) {
                crc16 ^= SWLP_CRC16_POLYNOM;
            }
        }
    }
    return crc16;
}

//  ***************************************************************************
/// @brief  Frame received callback
/// @param  frame_size: received frame size
/// @return none
//  ***************************************************************************
static void frame_received_callback(uint32_t frame_size) {
    state = STATE_FRAME_RECEIVED;
    received_frame_size = frame_size;
}

//  ***************************************************************************
/// @brief  Frame transmitter or error callback
/// @param  none
/// @return none
//  ***************************************************************************
static void frame_transmitted_or_error_callback(void) {
    state = STATE_WAIT_FRAME;
    usart2_start_rx();
}
