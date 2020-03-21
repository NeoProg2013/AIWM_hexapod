#ifndef SWLP_PROTOCOL_H
#define SWLP_PROTOCOL_H

#include <stdint.h>


#define SWLP_START_MARK_VALUE                           (0xAABBCCDD)
#define SWLP_CRC16_POLYNOM                              (0xA001)

//
// SWP command list
//
#define SWLP_CMD_NONE                                   (0x00)
#define SWLP_CMD_SELECT_SEQUENCE_UP                     (0x01)
#define SWLP_CMD_SELECT_SEQUENCE_DOWN                   (0x02)
#define SWLP_CMD_SELECT_SEQUENCE_DIRECT                 (0x03)
#define SWLP_CMD_SELECT_SEQUENCE_REVERSE                (0x04)
#define SWLP_CMD_SELECT_SEQUENCE_NONE                   (0x90)

//
// SWLP command status
//
#define SWLP_CMD_STATUS_ERROR                           (0x00)
#define SWLP_CMD_STATUS_OK                              (0x01)


#pragma pack(push, 1)
struct swlp_frame_t {
	uint32_t start_mark;
	uint8_t  payload[18];
	uint16_t crc16;
};

struct swlp_command_payload_t {
    uint8_t command;
    uint8_t step_length;
    int16_t curvature;
    uint8_t reserved[14];
};

struct swlp_status_payload_t {
	uint8_t  command;
	uint8_t  command_status;
	uint8_t  module_status;
	uint8_t  system_status;
	uint16_t battery_cell_voltage[3];
	uint16_t battery_voltage;
	uint8_t  battery_charge;
	uint8_t reserved[5];
};
#pragma pack(pop)


#endif // SWLP_PROTOCOL_H
