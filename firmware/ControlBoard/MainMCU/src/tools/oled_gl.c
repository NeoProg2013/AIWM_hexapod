//  ***************************************************************************
/// @file    oled_gl.c
/// @author  NeoProg
//  ***************************************************************************
#include "oled_gl.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "ssd1306_128x64.h"
#include "oled_gl_font_6x8.h"
#include "systimer.h"
#include "system_monitor.h"


typedef enum {
    STATE_NOINIT,
    STATE_IDLE,
    STATE_UPDATE_ROW,
    STATE_WAIT
} driver_state_t;


static driver_state_t driver_state = STATE_NOINIT;


//  ***************************************************************************
/// @brief  Graphic library initialization
/// @return none
//  ***************************************************************************
bool oled_gl_init(void) {
    if (!ssd1306_128x64_init())             return false;
    if (!ssd1306_128x64_set_inverse(false)) return false;
    if (!ssd1306_128x64_set_contrast(0xFF)) return false;
    if (!ssd1306_128x64_set_state(true))    return false;
    
    driver_state = STATE_IDLE;
    return true;
}

//  ***************************************************************************
/// @brief  Graphic library process
/// @return none
//  ***************************************************************************
bool oled_gl_process(void) {
    static uint32_t current_row = 0;
    
    switch (driver_state) {
        case STATE_IDLE:
            break;
            
        case STATE_UPDATE_ROW:
            if (ssd1306_128x64_start_async_update_row(current_row) == false) {
                return false;
            }
            
            ++current_row;
            if (current_row >= DISPLAY_MAX_ROW_COUNT) {
                current_row = 0;
                driver_state = STATE_IDLE;
                break;
            }
            
            driver_state = STATE_WAIT;
            break;
            
        case STATE_WAIT:
            if (ssd1306_128x64_is_async_operation_complete() == true) {
                if (ssd1306_128x64_is_async_operation_success() == false) {
                    return false;
                }
                driver_state = STATE_UPDATE_ROW;
            }
            break;
            
        case STATE_NOINIT:
        default:
            return false;
    }
    return true;
}

//  ***************************************************************************
/// @brief  Clear row fragment
/// @param  row: display row [0; 7]
/// @param  x, y: left top angle of rectangle (relative row)
/// @param  width, height: rectangle size (relative row)
/// @return none
//  ***************************************************************************
void oled_gl_clear_row_fragment(uint32_t row, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    uint8_t mask = 0;
    int32_t cursor = height - 1;
    while (cursor >= 0) {
        mask |= (1 << (y + cursor));
        --cursor;
    }
    
    uint8_t* frame_buffer = ssd1306_128x64_get_frame_buffer(row, x);
    for (uint32_t i = 0; i < width; ++i) {
        frame_buffer[i] &= ~mask;
    }
}

//  ***************************************************************************
/// @brief  Clear display
/// @param  none
/// @return none
//  ***************************************************************************
void oled_gl_clear_display(void) {
    for (uint32_t i = 0; i < 8; ++i) {
        uint8_t* frame_buffer = ssd1306_128x64_get_frame_buffer(i, 0);
        memset(frame_buffer, 0x00, 128);
    }
}

//  ***************************************************************************
/// @brief  Draw float number in format XX.X or X.XX
/// @param  row: display row [0; 7]
/// @param  x: first symbol position
/// @param  number: number for draw
/// @return none
//  ***************************************************************************
void oled_gl_draw_float_number(uint32_t row, uint32_t x, float number) {
    char buffer[12] = {0};
    sprintf(buffer, "%04.1f", number);
    oled_gl_draw_string(row, x, buffer);
}

//  ***************************************************************************
/// @brief  Draw number in DEC format
/// @param  row: display row [0; 7]
/// @param  x: first symbol position
/// @param  number: number for draw
/// @return none
//  ***************************************************************************
void oled_gl_draw_dec_number(uint32_t row, uint32_t x, int32_t number) {
    char buffer[12] = {0};
    sprintf(buffer, "%d", (int)number);
    oled_gl_draw_string(row, x, buffer);
}

//  ***************************************************************************
/// @brief  Draw number in HEX format (0xXXXX)
/// @param  row: display row [0; 7]
/// @param  x: first symbol position
/// @param  number: number for draw
/// @return none
//  ***************************************************************************
void oled_gl_draw_hex16(uint32_t row, uint32_t x, uint32_t number) {
    char buffer[32] = {0};
    sprintf(buffer, "0x%04X", (int)number);
    oled_gl_draw_string(row, x, buffer);
}

//  ***************************************************************************
/// @brief  Draw number in HEX format (0xXXXXXXXX)
/// @param  row: display row [0; 7]
/// @param  x: first symbol position
/// @param  number: number for draw
/// @return none
//  ***************************************************************************
void oled_gl_draw_hex32(uint32_t row, uint32_t x, uint32_t number) {
    char buffer[32] = {0};
    sprintf(buffer, "0x%08X", (int)number);
    oled_gl_draw_string(row, x, buffer);
}

//  ***************************************************************************
/// @brief  Draw string
/// @param  row: display row [0; 7]
/// @param  x: first symbol position
/// @param  str: string for draw
/// @return none
//  ***************************************************************************
void oled_gl_draw_string(uint32_t row, uint32_t x, const char* str) {
    uint8_t* frame_buffer = ssd1306_128x64_get_frame_buffer(row, x);
    for (uint32_t i = 0; *str != '\0'; ++str, i += 6) {
        memcpy(&frame_buffer[i], &font_6x8[(*str - ' ') * 6], 6);
    }
}

//  ***************************************************************************
/// @brief  Draw horizontal line
/// @param  row: display row [0; 7]
/// @param  x, y: line begin position (relative row)
/// @param  width: line width
/// @return none
//  ***************************************************************************
void oled_gl_draw_horizontal_line(uint32_t row, uint32_t x, uint32_t y, uint32_t width) {
    uint8_t* frame_buffer = ssd1306_128x64_get_frame_buffer(row, x);
    uint8_t mask = (1 << y);
    
    if (x + width > DISPLAY_WIDTH) {
        width -= DISPLAY_WIDTH - (x + width);
    }
    
    for (uint32_t i = 0; i < width; ++i) {
        frame_buffer[i] |= mask;
    }
}

//  ***************************************************************************
/// @brief  Draw rectangle
/// @param  row: display row [0; 7]
/// @param  x, y: left top angle of rectangle (relative row)
/// @param  width, height: rectangle size (relative row)
/// @return none
//  ***************************************************************************
void oled_gl_draw_rect(uint32_t row, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    uint8_t mask = 0;
    int32_t cursor = height - 1;
    while (cursor >= 0) {
        mask |= (1 << (y + cursor));
        --cursor;
    }
    
    uint8_t* frame_buffer = ssd1306_128x64_get_frame_buffer(row, x);
    for (uint32_t i = 0; i < width; ++i) {
        frame_buffer[i] |= mask;
    }
}

//  ***************************************************************************
/// @brief  Draw bitmap
/// @note   Bitmap height should be divisible of 8
/// @param  row: display row [0; 7]
/// @param  x: bitmap position
/// @param  bitmap_width, bitmap_height: bitmap size
/// @param  bitmap: bitmap data
/// @return none
//  ***************************************************************************
bool oled_gl_draw_bitmap(uint32_t row, uint32_t x, uint32_t bitmap_width, uint32_t bitmap_height, const uint8_t* bitmap) {
    if ((bitmap_height % 8) != 0) {
        return false;
    }
    
    for (uint32_t i = 0; i < (bitmap_height / 8); ++i, bitmap += bitmap_width) {
        uint8_t* frame_buffer = ssd1306_128x64_get_frame_buffer(row + i, x);
        memcpy(frame_buffer, bitmap, bitmap_width);
    }
    return true;
}

//  ***************************************************************************
/// @brief  Synchronous display update
/// @param  none
/// @return none
//  ***************************************************************************
bool oled_gl_sync_display_update(void) {
    return ssd1306_128x64_full_update();
}

//  ***************************************************************************
/// @brief  Start asynchronous display update
/// @param  none
/// @return none
//  ***************************************************************************
void oled_gl_start_async_display_update(void) {
    if (driver_state != STATE_IDLE) {
        return;
    }   
    driver_state = STATE_UPDATE_ROW;
}
