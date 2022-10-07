/// ***************************************************************************
/// @file    swlp.c
/// @author  NeoProg
/// ***************************************************************************
#include "project-base.h"
#include "swlp.h"
#include "swlp-protocol.h"
#include "usart2.h"
#include "indication.h"
#include "system-monitor.h"
#include "servo-driver.h"
#include "motion-core.h"
#include "systimer.h"
#include <math.h>
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


/// ***************************************************************************
/// @brief  SWLP driver initialization
/// ***************************************************************************
void swlp_init(void) {
    usart2_callbacks_t callbacks;
    callbacks.frame_received_callback = frame_received_callback;
    callbacks.frame_transmitted_callback = frame_transmitted_or_error_callback;
    callbacks.frame_error_callback = frame_transmitted_or_error_callback;
    usart2_init(COMMUNICATION_BAUD_RATE, &callbacks);
    
    state = STATE_WAIT_FRAME;
    usart2_start_rx();
}

/// ***************************************************************************
/// @brief  Process routine
/// ***************************************************************************
void swlp_process(void) {
    static uint64_t frame_receive_time = 0; // We are start with SYSMON_CONN_LOST_ERROR error
    if (state == STATE_FRAME_RECEIVED) {
        uint8_t* tx_buffer = usart2_get_tx_buffer();
        uint8_t* rx_buffer = usart2_get_rx_buffer();

        // Check frame
        if (!check_frame(rx_buffer, received_frame_size)) {
            state = STATE_WAIT_FRAME;
            usart2_start_rx();
            return;
        }

        // Preparing
        const swlp_frame_t* swlp_rx_frame = (const swlp_frame_t*)rx_buffer;
        const swlp_request_t* request = (const swlp_request_t*)swlp_rx_frame->payload;

        swlp_frame_t* swlp_tx_frame = (swlp_frame_t*)tx_buffer;
        swlp_response_t* response = (swlp_response_t*)swlp_tx_frame->payload;
        memset(swlp_tx_frame, 0, sizeof(swlp_frame_t));
        
        // Process motion parameters
        ext_motion_t motion = {0};
        motion.cfg.speed = request->speed;
        motion.cfg.curvature = request->curvature;
        motion.cfg.distance = request->distance;
        motion.cfg.step_height = request->step_height;
        motion.ctrl = request->motion_ctrl;
        motion.surface_point.x = request->surface_point_x;
        motion.surface_point.y = request->surface_point_y;
        motion.surface_point.z = request->surface_point_z;
        motion.surface_rotate.x = request->surface_rotate_x;
        motion.surface_rotate.y = request->surface_rotate_y;
        motion.surface_rotate.z = request->surface_rotate_z;
        motion_core_move(&motion);

        // Prepare response
        response->module_status = sysmon_module_status;
        response->system_status = sysmon_system_status;
        response->battery_voltage = sysmon_battery_voltage;
        response->battery_charge = sysmon_battery_charge;
        
        // Gathering current motion surface
        motion = motion_core_get_motion();
        response->speed = motion.cfg.speed;
        response->curvature = motion.cfg.curvature;
        response->distance = motion.cfg.distance;
        response->step_height = motion.cfg.step_height;
        response->motion_ctrl = motion.ctrl;
        response->surface_point_x = (int16_t)motion.surface_point.x;
        response->surface_point_y = (int16_t)motion.surface_point.y;
        response->surface_point_z = (int16_t)motion.surface_point.z;
        response->surface_rotate_x = (int16_t)motion.surface_rotate.x;
        response->surface_rotate_y = (int16_t)motion.surface_rotate.y;
        response->surface_rotate_z = (int16_t)motion.surface_rotate.z;

        // Prepare response
        swlp_tx_frame->start_mark = SWLP_START_MARK_VALUE;
        swlp_tx_frame->version = SWLP_CURRENT_VERSION;
        swlp_tx_frame->crc16 = calculate_crc16((uint8_t*)swlp_tx_frame, sizeof(swlp_frame_t) - sizeof(swlp_tx_frame->crc16));
        
        // Transmit response
        state = STATE_TRANSMIT;
        usart2_start_tx(sizeof(swlp_frame_t));
        
        // Update frame receive time
        frame_receive_time = get_time_ms();
    }
    
    // Process communication timeout feature
    sysmon_clear_error(SYSMON_CONN_LOST);
    if (get_time_ms() - frame_receive_time > COMMUNICATION_TIMEOUT || frame_receive_time == 0) {
        sysmon_set_error(SYSMON_CONN_LOST);
    }
}





/// ***************************************************************************
/// @brief  Check SWP frame
/// @param  rx_buffer: frame
/// @param  frame_size: frame size
/// @return true - frame valid, false - frame invalid
/// ***************************************************************************
static bool check_frame(const uint8_t* rx_buffer, uint32_t frame_size) {
    if (frame_size != sizeof(swlp_frame_t)) { // Check frame size
        return false;
    }

    // Check frame CRC16
    uint16_t crc = calculate_crc16(rx_buffer, frame_size - 2);
    if (calculate_crc16(rx_buffer, frame_size) != 0) {
        return false;
    }

    // Check start mark and vesrion
    const swlp_frame_t* swlp_frame = (const swlp_frame_t*)rx_buffer;
    if (swlp_frame->start_mark != SWLP_START_MARK_VALUE || swlp_frame->version != SWLP_CURRENT_VERSION) {
        return false;
    }

    return true;
}

/// ***************************************************************************
/// @brief  Calculate frame CRC16
/// @param  frame: frame
/// @param  size: frame size
/// @return CRC16 value
/// ***************************************************************************
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

/// ***************************************************************************
/// @brief  Frame received callback
/// @param  frame_size: received frame size
/// @return none
/// ***************************************************************************
static void frame_received_callback(uint32_t frame_size) {
    state = STATE_FRAME_RECEIVED;
    received_frame_size = frame_size;
}

/// ***************************************************************************
/// @brief  Frame transmitter or error callback
/// @param  none
/// @return none
/// ***************************************************************************
static void frame_transmitted_or_error_callback(void) {
    state = STATE_WAIT_FRAME;
    usart2_start_rx();
}
