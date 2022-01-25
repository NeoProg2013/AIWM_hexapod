//  ***************************************************************************
/// @file    ssd1306-128x64.c
/// @author  NeoProg
//  ***************************************************************************
#include "ssd1306-128x64.h"
#include "i2c2.h"
#include <string.h>


#define DISPLAY_I2C_ADDRESS                 (0x3C << 1)

#define FRAME_BEGIN_DEAD_ZONE               (2)
#define FRAME_END_DEAD_ZONE                 (2)
#define FRAME_ROW_COUNT                     (DISPLAY_HEIGHT / 8)
#define FRAME_COLUMN_COUNT                  (FRAME_BEGIN_DEAD_ZONE + DISPLAY_WIDTH + FRAME_END_DEAD_ZONE)
#define FRAME_BUFFER_SIZE                   (FRAME_ROW_COUNT * FRAME_COLUMN_COUNT)

#define SET_CONTRAST                        (0x81)
#define SET_DISPLAY_RESUME                  (0xA4)
#define SET_DISPLAY_ALL_ON                  (0xA5)
#define SET_NORMAL_DISPLAY                  (0xA6)
#define SET_INVERT_DISPLAY                  (0xA7)
#define SET_DISPLAY_OFF                     (0xAE)
#define SET_DISPLAY_ON                      (0xAF)
#define SET_DISPLAY_OFFSET                  (0xD3)
#define SET_COM_PINS_MODE                   (0xDA)
#define     COM_PINS_MODE_ALTERNATE         (0x12)
#define SET_DISPLAY_CLOCK_DIV               (0xD5)
#define SET_MULTIPLEX                       (0xA8)
#define SET_PAGE_START                      (0xB0)
#define SET_LOW_COLUMN                      (0x00)
#define SET_HIGH_COLUMN                     (0x10)
#define SET_START_LINE                      (0x40)
#define SET_MEMORY_MODE                     (0x20)
#define     MEMORY_MODE_PAGE                (0x02)
#define SET_COM_SCAN_INC                    (0xC0)
#define SET_COM_SCAN_DEC                    (0xC8)
#define SET_SEG_REMAP_DISABLE               (0xA0)
#define SET_SEG_REMAP_ENABLE                (0xA1)
#define SET_DC_DC_SETTINGS                  (0x8D)
#define     DC_DC_SETTINGS_ENABLE           (0x14)
#define     DC_DC_SETTINGS_DISABLE          (0x10)


static uint8_t frame_buffer[FRAME_BUFFER_SIZE] = {0};
    
static bool ssd1306_send_command(uint8_t cmd, uint8_t data, bool is_data);
static bool ssd1306_send_bytes(uint8_t* data, uint32_t bytes_count);


//  ***************************************************************************
/// @brief  Display initialization
/// @param  none
/// @return true - success, false - error
//  ***************************************************************************
bool ssd1306_128x64_init(void) {
    i2c2_init(I2C_SPEED_400KHZ);
    
    // Disable display
    if (!ssd1306_128x64_set_state(false)) return false;
    
    // Base configuration
    if (!ssd1306_send_command(SET_DISPLAY_RESUME, 0x00, false)) return false;                  // Output follows RAM content

    // Addressing configuration
    if (!ssd1306_send_command(SET_MEMORY_MODE, MEMORY_MODE_PAGE, true)) return false;          // Set page addressing mode

    // Hardware configuration
    if (!ssd1306_send_command(SET_START_LINE, 0x00, false)) return false;                      // Set start line address = 0
    if (!ssd1306_send_command(SET_SEG_REMAP_ENABLE, 0x00, false)) return false;                // Set segment re-map: column address 0 is mapped to SEG127
    if (!ssd1306_send_command(SET_MULTIPLEX, 0x3F, true)) return false;                        // Select multiplex ratio register = 63 + 1
    if (!ssd1306_send_command(SET_COM_SCAN_DEC, 0x00, false)) return false;                    // Set COM Output Scan Direction = Reverse
    if (!ssd1306_send_command(SET_DISPLAY_OFFSET, 0x00, true)) return false;                   // Select display offset register = no offset
    if (!ssd1306_send_command(SET_COM_PINS_MODE, COM_PINS_MODE_ALTERNATE, true)) return false; // Select COM pin hardware configuration register

    // Timing and driving scheme configuration
    if (!ssd1306_send_command(SET_DISPLAY_CLOCK_DIV, 0x80, true)) return false;                // Select display clock divide ratio/oscillator frequency register
    
    // Clear buffers
    memset(frame_buffer, 0x00, sizeof(frame_buffer));
    return ssd1306_128x64_full_update();
}

//  ***************************************************************************
/// @brief  Set display contrast
/// @param  contrast: contrast value
/// @return true - success, false - error
//  ***************************************************************************
bool ssd1306_128x64_set_contrast(uint8_t contrast) {
    return ssd1306_send_command(SET_CONTRAST, contrast, true); // Set contrast
}

//  ***************************************************************************
/// @brief  Set display inverse
/// @param  is_inverse: true - inverse, false - normal
/// @return true - success, false - error
//  ***************************************************************************
bool ssd1306_128x64_set_inverse(bool is_inverse) {
    uint8_t cmd = (is_inverse == true) ? SET_INVERT_DISPLAY : SET_NORMAL_DISPLAY;
    return ssd1306_send_command(cmd, 0x00, false); // Enable display
}

//  ***************************************************************************
/// @brief  Display power control
/// @param  is_inverse: true - enable, false - disable
/// @return true - success, false - error
//  ***************************************************************************
bool ssd1306_128x64_set_state(bool is_enable) {
    if (is_enable == true) {
        if (!ssd1306_send_command(SET_DC_DC_SETTINGS, DC_DC_SETTINGS_ENABLE, true)) return false; // Enable DC-DC
        if (!ssd1306_send_command(SET_DISPLAY_ON, 0x00, false)) return false; // Enable display
    }
    else {
        if (!ssd1306_send_command(SET_DC_DC_SETTINGS, DC_DC_SETTINGS_DISABLE, true)) return false; // Disable DC-DC
        if (!ssd1306_send_command(SET_DISPLAY_OFF, 0x00, false)) return false; // Disable display
    }
    return true;
}

//  ***************************************************************************
/// @brief  Start asynchronous update row
/// @param  row: row index [0; 7]
/// @return true - success, false - error
//  ***************************************************************************
bool ssd1306_128x64_start_async_update_row(uint32_t row) {
    if (!ssd1306_send_command(SET_PAGE_START + row, 0x00, false)) return false;
    if (!ssd1306_send_command(SET_LOW_COLUMN, 0x00, false)) return false;
    if (!ssd1306_send_command(SET_HIGH_COLUMN, 0x00, false)) return false;
    
    uint8_t* row_buffer = &frame_buffer[row * FRAME_COLUMN_COUNT];
    return i2c2_async_write(DISPLAY_I2C_ADDRESS, 0x40, 1, row_buffer, FRAME_COLUMN_COUNT);
}

//  ***************************************************************************
/// @brief  Check asynchronous operation state
/// @param  none
/// @return true - operation complete, false - operation in progress
//  ***************************************************************************
bool ssd1306_128x64_is_async_operation_complete(void) {
    return i2c2_is_async_operation_completed();
}

//  ***************************************************************************
/// @brief  Check asynchronous operation status
/// @param  none
/// @return true - success, false - error
//  ***************************************************************************
bool ssd1306_128x64_is_async_operation_success(void) {
    return i2c2_get_async_operation_result();
}

//  ***************************************************************************
/// @brief  Transfer display frame
/// @param  none
/// @return true - success, false - error
//  ***************************************************************************
bool ssd1306_128x64_full_update(void) {
    for (uint32_t i = 0; i < FRAME_ROW_COUNT; ++i) {
        if (!ssd1306_send_command(SET_PAGE_START + i, 0x00, false)) return false;
        if (!ssd1306_send_command(SET_LOW_COLUMN, 0x00, false))     return false;
        if (!ssd1306_send_command(SET_HIGH_COLUMN, 0x00, false))    return false;
        
        if (!ssd1306_send_bytes(&frame_buffer[FRAME_COLUMN_COUNT * i], FRAME_COLUMN_COUNT)) return false;
    }
    return true;
}

//  ***************************************************************************
/// @brief  Get pointer to fragment of inactive display frame buffer
/// @param  row: row index [0; 7]
/// @param  column: column index [0; 127]
/// @return pointer to fragment of frame buffer
//  ***************************************************************************
uint8_t* ssd1306_128x64_get_frame_buffer(uint32_t row, uint32_t column) {
    return &frame_buffer[row * FRAME_COLUMN_COUNT + column + FRAME_BEGIN_DEAD_ZONE];
}


//  ***************************************************************************
/// @brief  Send command to display
/// @param  cmd: command
/// @return true - success, false - error
//  ***************************************************************************
static bool ssd1306_send_command(uint8_t cmd, uint8_t data, bool is_data) {
    if (is_data == true) {
        uint8_t tx_buffer[2];
        tx_buffer[0] = cmd;
        tx_buffer[1] = data;
        return i2c2_write(DISPLAY_I2C_ADDRESS, 0x00, 1, tx_buffer, 2); // 0x00 - Control byte = Command
    }
    return i2c2_write(DISPLAY_I2C_ADDRESS, 0x00, 1, &cmd, 1); // 0x00 - Control byte = Command
}

//  ***************************************************************************
/// @brief  Send bytes to display GRAM
/// @param  data: pointer to data
/// @param  bytes_count: bytes count
/// @return true - success, false - error
//  ***************************************************************************
static bool ssd1306_send_bytes(uint8_t* data, uint32_t bytes_count) {
    return i2c2_write(DISPLAY_I2C_ADDRESS, 0x40, 1, data, bytes_count); // 0x40 - Control byte = Data
}
