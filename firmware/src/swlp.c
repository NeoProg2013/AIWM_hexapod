//  ***************************************************************************
/// @file    swlp.c
/// @author  NeoProg
//  ***************************************************************************
#include "swlp.h"
#include "stm32f373xc.h"
#include "swlp_protocol.h"
#include "movement_engine.h"
#include "motion_core.h"
#include "indication.h"
#include "system_monitor.h"
#include <stdint.h>


static bool check_frame(const uint8_t* rx_buffer, uint32_t frame_size);
static uint16_t calculate_crc16(const uint8_t* frame, uint32_t size);


//  ***************************************************************************
/// @brief  SWLP driver initialization
/// @param  none
/// @return none
//  ***************************************************************************
void swlp_init(void) {

}

//  ***************************************************************************
/// @brief  Process received frame
/// @param  rx_buffer: received data
/// @param  frame_size: frame size
/// @param  tx_buffer: transmit data
/// @retval tx_buffer
/// @return bytes for transmit from tx_buffer
//  ***************************************************************************
uint32_t swlp_process_frame(const uint8_t* rx_buffer, uint32_t frame_size, uint8_t* tx_buffer) {

    // Check frame
    if (check_frame(rx_buffer, frame_size) == false) {
        return 0;
    }

    // Preparing
    const swlp_frame_t* swlp_rx_frame = (const swlp_frame_t*)rx_buffer;
    const swlp_command_payload_t* request = (const swlp_command_payload_t*)swlp_rx_frame->payload;

    swlp_frame_t* swlp_tx_frame = (swlp_frame_t*)tx_buffer;
    swlp_status_payload_t* response = (swlp_status_payload_t*)swlp_tx_frame->payload;

    // Process command
    response->command_status = SWLP_CMD_STATUS_OK;
    switch (request->command) {

        case SWLP_CMD_NONE:
            break;
        case SWLP_CMD_SELECT_SEQUENCE_UP:
            movement_engine_select_sequence(SEQUENCE_UP);
            break;
        case SWLP_CMD_SELECT_SEQUENCE_DOWN:
            movement_engine_select_sequence(SEQUENCE_DOWN);
            break;
        case SWLP_CMD_SELECT_SEQUENCE_DIRECT:
            movement_engine_select_sequence(SEQUENCE_DIRECT);
            motion_core_update_trajectory_config(request->curvature, request->step_length);
            break;
        case SWLP_CMD_SELECT_SEQUENCE_REVERSE:
            movement_engine_select_sequence(SEQUENCE_REVERSE);
            motion_core_update_trajectory_config(request->curvature, request->step_length);
            break;
        
        //case SWLP_CMD_SELECT_SEQUENCE_RUN:                   movement_engine_select_sequence(SEQUENCE_RUN);                   break;
        //case SWLP_CMD_SELECT_SEQUENCE_DIRECT_MOVEMENT:       movement_engine_select_sequence(SEQUENCE_DIRECT_MOVEMENT);       break;
        //case SWLP_CMD_SELECT_SEQUENCE_REVERSE_MOVEMENT:      movement_engine_select_sequence(SEQUENCE_REVERSE_MOVEMENT);      break;
        //case SWLP_CMD_SELECT_SEQUENCE_ROTATE_LEFT:           movement_engine_select_sequence(SEQUENCE_ROTATE_LEFT);           break;
        //case SWLP_CMD_SELECT_SEQUENCE_ROTATE_RIGHT:          movement_engine_select_sequence(SEQUENCE_ROTATE_RIGHT);          break;
        //case SWLP_CMD_SELECT_SEQUENCE_DIRECT_MOVEMENT_SLOW:  movement_engine_select_sequence(SEQUENCE_DIRECT_MOVEMENT_SLOW);  break;
        //case SWLP_CMD_SELECT_SEQUENCE_REVERSE_MOVEMENT_SLOW: movement_engine_select_sequence(SEQUENCE_REVERSE_MOVEMENT_SLOW); break;
        //case SWLP_CMD_SELECT_SEQUENCE_SHIFT_LEFT:            movement_engine_select_sequence(SEQUENCE_SHIFT_LEFT);            break;
        //case SWLP_CMD_SELECT_SEQUENCE_SHIFT_RIGHT:           movement_engine_select_sequence(SEQUENCE_SHIFT_RIGHT);           break;
        //case SWLP_CMD_SELECT_SEQUENCE_ATTACK_LEFT:           movement_engine_select_sequence(SEQUENCE_ATTACK_LEFT);           break;
        //case SWLP_CMD_SELECT_SEQUENCE_ATTACK_RIGHT:          movement_engine_select_sequence(SEQUENCE_ATTACK_RIGHT);          break;
        //case SWLP_CMD_SELECT_SEQUENCE_DANCE:                 movement_engine_select_sequence(SEQUENCE_DANCE);                 break;
        //case SWLP_CMD_SELECT_SEQUENCE_ROTATE_X:              movement_engine_select_sequence(SEQUENCE_ROTATE_X);              break;
        ////case SWLP_CMD_SELECT_SEQUENCE_ROTATE_Y:              movement_engine_select_sequence(SEQUENCE_ROTATE_Y);              break;
        //case SWLP_CMD_SELECT_SEQUENCE_ROTATE_Z:              movement_engine_select_sequence(SEQUENCE_ROTATE_Z);              break;
        case SWLP_CMD_SELECT_SEQUENCE_NONE:
            movement_engine_select_sequence(SEQUENCE_NONE);
            break;
        
        //case SWLP_CMD_SWITCH_LIGHT:                          indication_switch_light_state();                                 break;                                            break;

        default:
            response->command_status = SWLP_CMD_STATUS_ERROR;
    }

    // Prepare status payload
    response->command = request->command;
    response->module_status = sysmon_module_status;
    response->system_status = sysmon_system_status;
    response->battery_cell_voltage[0] = sysmon_battery_cell_voltage[0];
    response->battery_cell_voltage[1] = sysmon_battery_cell_voltage[1];
    response->battery_cell_voltage[2] = sysmon_battery_cell_voltage[2];
    response->battery_voltage = sysmon_battery_voltage;
    response->battery_charge = sysmon_battery_charge;

    // Prepare response
    swlp_tx_frame->start_mark = SWLP_START_MARK_VALUE;
    swlp_tx_frame->crc16 = calculate_crc16((uint8_t*)swlp_tx_frame, sizeof(swlp_frame_t) - 2);

    return sizeof(swlp_frame_t);
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
