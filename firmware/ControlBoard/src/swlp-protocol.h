/// ***************************************************************************
/// @file    swlp_protocol.h
/// @author  NeoProg
/// @brief   Simple wireless protocol definition
/// ***************************************************************************
#ifndef _SWLP_PROTOCOL_H_
#define _SWLP_PROTOCOL_H_

#define SWLP_START_MARK_VALUE           (0xAABBCCDD)
#define SWLP_CURRENT_VERSION            (0x03)
#define SWLP_CRC16_POLYNOM              (0xA001)

// Motion ctrl flags
#define SWLP_MOTION_CTRL_NO             (0x0000u)
#define SWLP_MOTION_CTRL_EN_STAB        (0x0001u)


#pragma pack(push, 1)
typedef struct {
    uint32_t start_mark;
    uint8_t  version;
    uint8_t  payload[25];
    uint16_t crc16;
} swlp_frame_t;

typedef struct {
    // Basic motion parameters
    uint8_t speed;
    int16_t curvature;
    int8_t  distance;
    uint8_t step_height;
    // Advanced motion parameters
    uint16_t motion_ctrl;
    int16_t surface_point_x;
    int16_t surface_point_y;
    int16_t surface_point_z;
    int16_t surface_rotate_x;
    int16_t surface_rotate_y;
    int16_t surface_rotate_z;
    uint8_t reserved[6];
} swlp_request_t;

typedef struct {
    uint8_t  module_status;
    uint8_t  system_status;
    uint16_t battery_voltage;
    uint8_t  battery_charge;
    // Actual values of basic motion parameters
    uint8_t speed;
    int16_t curvature;
    int8_t  distance;
    uint8_t step_height;
    // Actual values of advanced motion parameters
    uint16_t motion_ctrl;
    int16_t surface_point_x;
    int16_t surface_point_y;
    int16_t surface_point_z;
    int16_t surface_rotate_x;
    int16_t surface_rotate_y;
    int16_t surface_rotate_z;
    uint8_t reserved[1];
} swlp_response_t;
#pragma pack(pop)


static_assert(sizeof(swlp_request_t) == 25, "size of swlp_request_t is not equal size of swlp_frame_t::payload");
static_assert(sizeof(swlp_response_t) == 25, "size of swlp_response_t is not equal size of swlp_frame_t::payload");

#endif // _SWLP_PROTOCOL_H_
