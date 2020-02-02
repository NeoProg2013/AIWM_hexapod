//  ***************************************************************************
/// @file    swp.c
/// @author  NeoProg
//  ***************************************************************************
#include "swp.h"
#include "swp_protocol.h"
#include "system_monitor.h"
#include <stdint.h>


static bool check_frame(const uint8_t* rx_buffer, uint32_t frame_size);
static uint16_t calculate_crc16(const uint8_t* frame, uint32_t size);


//  ***************************************************************************
/// @brief  SWP driver initialization
/// @param  none
/// @return none
//  ***************************************************************************
void swp_init(void) {


}

//  ***************************************************************************
/// @brief  Process received frame
/// @param  rx_buffer: received data
/// @param  frame_size: frame size
/// @param  tx_buffer: transmit data
/// @retval tx_buffer
/// @return bytes for transmit from tx_buffer
//  ***************************************************************************
uint32_t swp_process_frame(const uint8_t* rx_buffer, uint32_t frame_size, uint8_t* tx_buffer) {

    // Check frame
    if (check_frame(rx_buffer, frame_size) == false) {
        return 0;
    }

    // Preparing
    const swp_frame_t* swp_rx_frame = (const swp_frame_t*)rx_buffer;
    const swp_request_payload_t* request = (const swp_request_payload_t*)swp_rx_frame->payload;

    swp_frame_t* swp_tx_frame = (swp_frame_t*)tx_buffer;
    swp_response_payload_t* response = (swp_response_payload_t*)swp_tx_frame->payload;

    // Process command
    switch (request->command) {

        /*case SCR_CMD_SELECT_SEQUENCE_UP:
            movement_engine_select_sequence(SEQUENCE_UP);
            break;

        case SCR_CMD_SELECT_SEQUENCE_DOWN:
            movement_engine_select_sequence(SEQUENCE_DOWN);
            break;

        case SCR_CMD_SELECT_SEQUENCE_RUN:
            movement_engine_select_sequence(SEQUENCE_RUN);
            break;

        case SCR_CMD_SELECT_SEQUENCE_DIRECT_MOVEMENT:
            movement_engine_select_sequence(SEQUENCE_DIRECT_MOVEMENT);
            break;

        case SCR_CMD_SELECT_SEQUENCE_REVERSE_MOVEMENT:
            movement_engine_select_sequence(SEQUENCE_REVERSE_MOVEMENT);
            break;

        case SCR_CMD_SELECT_SEQUENCE_ROTATE_LEFT:
            movement_engine_select_sequence(SEQUENCE_ROTATE_LEFT);
            break;

        case SCR_CMD_SELECT_SEQUENCE_ROTATE_RIGHT:
            movement_engine_select_sequence(SEQUENCE_ROTATE_RIGHT);
            break;

        case SCR_CMD_SELECT_SEQUENCE_DIRECT_MOVEMENT_SLOW:
            movement_engine_select_sequence(SEQUENCE_DIRECT_MOVEMENT_SLOW);
            break;

        case SCR_CMD_SELECT_SEQUENCE_REVERSE_MOVEMENT_SLOW:
            movement_engine_select_sequence(SEQUENCE_REVERSE_MOVEMENT_SLOW);
            break;

        case SCR_CMD_SELECT_SEQUENCE_SHIFT_LEFT:
            movement_engine_select_sequence(SEQUENCE_SHIFT_LEFT);
            break;

        case SCR_CMD_SELECT_SEQUENCE_SHIFT_RIGHT:
            movement_engine_select_sequence(SEQUENCE_SHIFT_RIGHT);
            break;



        case SCR_CMD_SELECT_SEQUENCE_ATTACK_LEFT:
            movement_engine_select_sequence(SEQUENCE_ATTACK_LEFT);
            break;

        case SCR_CMD_SELECT_SEQUENCE_ATTACK_RIGHT:
            movement_engine_select_sequence(SEQUENCE_ATTACK_RIGHT);
            break;

        case SCR_CMD_SELECT_SEQUENCE_DANCE:
            movement_engine_select_sequence(SEQUENCE_DANCE);
            break;

        case SCR_CMD_SELECT_SEQUENCE_ROTATE_X:
            movement_engine_select_sequence(SEQUENCE_ROTATE_X);
            break;

        //case SCR_CMD_SELECT_SEQUENCE_ROTATE_Y:
            //movement_engine_select_sequence(SEQUENCE_ROTATE_Y);
            //break;

        case SCR_CMD_SELECT_SEQUENCE_ROTATE_Z:
            movement_engine_select_sequence(SEQUENCE_ROTATE_Z);
            break;



        case SCR_CMD_SELECT_SEQUENCE_NONE:
            movement_engine_select_sequence(SEQUENCE_NONE);
            break;



        case SCR_CMD_RESET:
            //REG_RSTC_CR = 0xA5000005;
            break;*/

        default:
            return 0;
    }

    // Prepare status payload
    response->command = request->command;
    response->module_status = sysmon_module_status;
    response->system_status = sysmon_system_status;
    response->battery_cell_voltage[0] = sysmon_battery_cell_voltage[0];
    response->battery_cell_voltage[1] = sysmon_battery_cell_voltage[1];
    response->battery_cell_voltage[2] = sysmon_battery_cell_voltage[2];

    // Prepare response
    swp_tx_frame->start_mark = SWP_START_MARK_VALUE;
    swp_tx_frame->frame_number = swp_rx_frame->frame_number;
    swp_tx_frame->crc16 = calculate_crc16((uint8_t*)swp_tx_frame, sizeof(swp_frame_t));

    return sizeof(swp_frame_t);
}





//  ***************************************************************************
/// @brief  Check SWP frame
/// @param  rx_buffer: frame
/// @param  frame_size: frame size
/// @return true - frame valid, false - frame invalid
//  ***************************************************************************
static bool check_frame(const uint8_t* rx_buffer, uint32_t frame_size) {

    // Check frame size
    if (frame_size != sizeof(swp_frame_t)) {
        return false;
    }

    // Check frame CRC16
    if (calculate_crc16(rx_buffer, frame_size) != 0) {
        return false;
    }

    // Check start mark
    const swp_frame_t* swp_frame = (const swp_frame_t*)rx_buffer;
    if (swp_frame->start_mark != SWP_START_MARK_VALUE) {
        return false;
    }

    return true;
}

//  ***************************************************************************
/// @brief  Calculate ModBus frame CRC16
/// @param  frame: ModBus frame
/// @param  size:  frame size
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
                crc16 ^= SWP_CRC16_POLYNOM;
            }
        }
    }
    return crc16;
}
