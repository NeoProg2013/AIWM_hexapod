/// ***************************************************************************
/// @file    swlp_protocol.h
/// @author  NeoProg
/// @brief   Simple wireless protocol definition
/// ***************************************************************************
#ifndef _SWLP_PROTOCOL_H_
#define _SWLP_PROTOCOL_H_

#include <stdint.h>


#define SWLP_START_MARK_VALUE                           (0xAABBCCDD)
#define SWLP_CURRENT_VERSION                            (0x02)
#define SWLP_CRC16_POLYNOM                              (0xA001)

//
// SWP command list
//
#define SWLP_CMD_NONE                                   (0x00)
#define SWLP_CMD_SELECT_SCRIPT_UP                       (0x01)
#define SWLP_CMD_SELECT_SCRIPT_DOWN                     (0x02)
#define SWLP_CMD_SELECT_SCRIPT_X_ROTATE                 (0x03)
#define SWLP_CMD_SELECT_SCRIPT_Z_ROTATE                 (0x04)
#define SWLP_CMD_SELECT_SCRIPT_XY_ROTATE                (0x05)
#define SWLP_CMD_SELECT_SCRIPT_UP_DOWN                  (0x06)
#define SWLP_CMD_SELECT_SCRIPT_Z_PUSH_PULL              (0x07)
#define SWLP_CMD_SELECT_SCRIPT_X_SWAY                   (0x08)
#define SWLP_CMD_SELECT_SCRIPT_SQUARE                   (0x09)
#define SWLP_CMD_MOVE                                   (0x80)

//
// SWLP command status
//
#define SWLP_CMD_STATUS_ERROR                           (0x00)
#define SWLP_CMD_STATUS_OK                              (0x01)


#pragma pack(push, 1)
typedef struct {
    uint32_t start_mark;
    uint8_t  version;
    uint8_t  payload[25];
    uint16_t crc16;
} swlp_frame_t;

typedef struct {
    uint8_t command;
    uint8_t speed;
    int16_t curvature;
    int8_t  distance;
    uint8_t step_height;
    int16_t surface_point_x;
    int16_t surface_point_y;
    int16_t surface_point_z;
    int16_t surface_rotate_x;
    int16_t surface_rotate_y;
    int16_t surface_rotate_z;
    uint8_t reserved[7];
} swlp_command_payload_t;

typedef struct {
    uint8_t  command;
    uint8_t  command_status;
    uint8_t  module_status;
    uint8_t  system_status;
    uint16_t battery_voltage;
    uint8_t  battery_charge;
    int16_t  surface_point_x;
    int16_t  surface_point_y;
    int16_t  surface_point_z;
    int16_t  surface_rotate_x;
    int16_t  surface_rotate_y;
    int16_t  surface_rotate_z;
    uint8_t  reserved[6];
} swlp_status_payload_t;
#pragma pack(pop)


static_assert(sizeof(swlp_command_payload_t) == 25, "size of swlp_command_payload_t is not equal size of swlp_frame_t::payload");
static_assert(sizeof(swlp_status_payload_t) == 25, "size of swlp_status_payload_t is not equal size of swlp_frame_t::payload");

#endif // _SWLP_PROTOCOL_H_
