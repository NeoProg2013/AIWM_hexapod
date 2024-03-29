/// ***************************************************************************
/// @file    ssd1306-128x64.h
/// @author  NeoProg
/// @brief   SSD1306 128x64 display driver
/// ***************************************************************************
#ifndef _SSD1306_128X64_H_
#define _SSD1306_128X64_H_
#define DISPLAY_WIDTH                   (128)
#define DISPLAY_HEIGHT                  (64)
#define DISPLAY_MAX_ROW_COUNT           (8)


extern bool ssd1306_128x64_init(void);
extern bool ssd1306_128x64_set_contrast(uint8_t contrast);
extern bool ssd1306_128x64_set_inverse(bool is_inverse);
extern bool ssd1306_128x64_set_state(bool is_enable);
extern bool ssd1306_128x64_start_row_update(uint32_t row);
extern bool ssd1306_128x64_send_row_data(uint32_t row, uint32_t count);
extern bool ssd1306_128x64_is_row_updated(void);
extern bool ssd1306_128x64_full_update(void);
extern uint8_t* ssd1306_128x64_get_frame_buffer(uint32_t row, uint32_t column);


#endif // _SSD1306_128X64_H_
