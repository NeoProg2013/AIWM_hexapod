//  ***************************************************************************
/// @file    swlp_protocol.h
/// @author  NeoProg
/// @brief   Simple wireless protocol definition
//  ***************************************************************************
#ifndef _SWLP_PROTOCOL_H_
#define _SWLP_PROTOCOL_H_

#include <stdint.h>


#define SWLP_START_MARK_VALUE                           (0xAABBCCDD)
#define SWLP_CRC16_POLYNOM                              (0xA001)

//
// SWP command list
//
#define SWLP_CMD_NONE                                   (0x00)
#define SWLP_CMD_SELECT_SEQUENCE_UP                     (0x01)
#define SWLP_CMD_SELECT_SEQUENCE_DOWN                   (0x02)
#define SWLP_CMD_SELECT_SEQUENCE_RUN                    (0x03)
#define SWLP_CMD_SELECT_SEQUENCE_DIRECT_MOVEMENT        (0x04)
#define SWLP_CMD_SELECT_SEQUENCE_REVERSE_MOVEMENT       (0x05)
#define SWLP_CMD_SELECT_SEQUENCE_ROTATE_LEFT            (0x06)
#define SWLP_CMD_SELECT_SEQUENCE_ROTATE_RIGHT           (0x07)
#define SWLP_CMD_SELECT_SEQUENCE_DIRECT_MOVEMENT_SLOW   (0x08)
#define SWLP_CMD_SELECT_SEQUENCE_REVERSE_MOVEMENT_SLOW  (0x09)
#define SWLP_CMD_SELECT_SEQUENCE_SHIFT_LEFT             (0x10)
#define SWLP_CMD_SELECT_SEQUENCE_SHIFT_RIGHT            (0x11)
#define SWLP_CMD_SELECT_SEQUENCE_ATTACK_LEFT            (0x20)
#define SWLP_CMD_SELECT_SEQUENCE_ATTACK_RIGHT           (0x21)
#define SWLP_CMD_SELECT_SEQUENCE_DANCE                  (0x30)
#define SWLP_CMD_SELECT_SEQUENCE_ROTATE_X               (0x31)
//#define SWLP_CMD_SELECT_SEQUENCE_ROTATE_Y               (0x32)
#define SWLP_CMD_SELECT_SEQUENCE_ROTATE_Z               (0x33)
#define SWLP_CMD_SELECT_SEQUENCE_NONE                   (0x90)

#define SWLP_CMD_SWITCH_LIGHT                           (0xA0)

#define SWLP_CMD_RESET                                  (0xFE)

//
// SWLP command status
//
#define SWLP_CMD_STATUS_ERROR                           (0x00)
#define SWLP_CMD_STATUS_OK                              (0x01)


#pragma pack(push, 1)
typedef struct {
    uint32_t start_mark;
    uint8_t  payload[18];
    uint16_t crc16;
} swlp_frame_t;

typedef struct {
    uint8_t command;
    uint8_t reserved[17];
} swlp_command_payload_t;

typedef struct {
    uint8_t  command;
    uint8_t  command_status;
    uint8_t  module_status;
    uint8_t  system_status;
    uint16_t battery_cell_voltage[3];
    uint16_t battery_voltage;
	uint8_t  battery_charge;
	uint8_t  reserved[5];
} swlp_status_payload_t;
#pragma pack(pop)


#endif // _SWLP_PROTOCOL_H_
