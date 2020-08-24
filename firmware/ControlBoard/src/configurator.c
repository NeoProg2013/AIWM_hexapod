//  ***************************************************************************
/// @file    configurator.c
/// @author  NeoProg
//  ***************************************************************************
#include "configurator.h"
#include "project_base.h"
#include "memory_map.h"
#include "i2c1.h"
#include "system_monitor.h"
#include "systimer.h"

#define STORAGE_DEVICE_ADDRESS              (0xA0)

#define CONFIG_SECTION_ADDRESS              (0x0000)
#define CONFIG_SECTION_PAGE_SIZE            (256)
#define CONFIG_SECTION_PAGE_COUNT           (3)
#define CONFIG_SECTION_SIZE                 (CONFIG_SECTION_PAGE_SIZE * CONFIG_SECTION_PAGE_COUNT)

#define CONFIG_SECTION_MAX_READ_SIZE        (128)
#define CONFIG_SECTION_MAX_WRITE_SIZE       (32)


static uint8_t cli_buffer[CONFIG_SECTION_PAGE_SIZE] = {0};


static bool config_calc_checksum(uint32_t page, uint32_t *checksum);
static bool config_verify_checksum(uint32_t page);
static bool config_erase(void);


//  ***************************************************************************
/// @brief  Configurator initialization
/// @param  none
/// @return none
//  ***************************************************************************
void config_init(void) {
    
    i2c1_init(I2C_SPEED_400KHZ);

    /*for (uint32_t page = 0; page < CONFIG_SECTION_PAGE_COUNT; ++page) {

        if (config_verify_checksum(page) == false) {
            sysmon_set_error(SYSMON_MEMORY_ERROR);
            sysmon_disable_module(SYSMON_MODULE_CONFIGURATOR);
            return;
        }
    }*/
}

//  ***************************************************************************
/// @brief  Read data from configuration section
/// @param  address: data block address
/// @param  buffer: buffer for data
/// @param  bytes_count: bytes count for read
/// @return true - success, false - error
//  ***************************************************************************
bool config_read(uint32_t address, uint8_t* buffer, uint32_t bytes_count) {

    // Clear buffer
    memset(buffer, 0xFF, bytes_count);
    
    if (sysmon_is_module_disable(SYSMON_MODULE_CONFIGURATOR) == true) {
        return false;
    }


    if (address + bytes_count > CONFIG_SECTION_SIZE) {
        sysmon_set_error(SYSMON_FATAL_ERROR);
        sysmon_disable_module(SYSMON_MODULE_CONFIGURATOR);
        return false;
    }

    while (bytes_count != 0) {
        
        uint32_t block_size = (bytes_count < CONFIG_SECTION_MAX_READ_SIZE) ? bytes_count : CONFIG_SECTION_MAX_READ_SIZE;
   
        if (i2c1_read(STORAGE_DEVICE_ADDRESS, address, 2, buffer, block_size) == false) {
            sysmon_set_error(SYSMON_I2C_ERROR | SYSMON_MEMORY_ERROR);
            sysmon_disable_module(SYSMON_MODULE_CONFIGURATOR);
            return false;
        }
        
        bytes_count -= block_size;
        address += bytes_count;
        buffer += bytes_count;
    }
    
    return true;
}

//  ***************************************************************************
/// @brief  Write data to configuration section
/// @param  address: data block address
/// @param  data: data for write
/// @param  bytes_count: bytes count for write
/// @return true - success, false - error
//  ***************************************************************************
bool config_write(uint32_t address, uint8_t* data, uint32_t bytes_count) {

    if (sysmon_is_module_disable(SYSMON_MODULE_CONFIGURATOR) == true) {
        return false;
    }


    if (address + bytes_count > CONFIG_SECTION_SIZE) {
        sysmon_set_error(SYSMON_FATAL_ERROR);
        sysmon_disable_module(SYSMON_MODULE_CONFIGURATOR);
        return false;
    }
    
    while (bytes_count != 0) {
        
        uint32_t block_size = (bytes_count < CONFIG_SECTION_MAX_WRITE_SIZE) ? bytes_count : CONFIG_SECTION_MAX_WRITE_SIZE;
   
        if (i2c1_write(STORAGE_DEVICE_ADDRESS, address, 2, data, block_size) == false) {
            sysmon_set_error(SYSMON_I2C_ERROR | SYSMON_MEMORY_ERROR);
            sysmon_disable_module(SYSMON_MODULE_CONFIGURATOR);
            return false;
        }
        delay_ms(10);
        
        bytes_count -= block_size;
        address += bytes_count;
        data += bytes_count;
    }
    return true;
}

//  ***************************************************************************
/// @brief  Read BYTE/WORD/DWORD from configuration section
/// @param  address: data block address
/// @param  buffer: buffer for data
/// @return true - success, false - error
//  ***************************************************************************
bool config_read_8(uint32_t address, uint8_t* buffer)   { return config_read(address, (uint8_t*)buffer, 1); }
bool config_read_16(uint32_t address, uint16_t* buffer) { return config_read(address, (uint8_t*)buffer, 2); }
bool config_read_32(uint32_t address, uint32_t* buffer) { return config_read(address, (uint8_t*)buffer, 4); }

//  ***************************************************************************
/// @brief  Write BYTE/WORD/DWORD to configuration section
/// @param  address: data block address
/// @param  data: data for write
/// @return true - success, false - error
//  ***************************************************************************
bool config_write_8(uint32_t address, uint8_t data)   { return config_write(address, (uint8_t*)&data, 1); }
bool config_write_16(uint32_t address, uint16_t data) { return config_write(address, (uint8_t*)&data, 2); }
bool config_write_32(uint32_t address, uint32_t data) { return config_write(address, (uint8_t*)&data, 4); }

//  ***************************************************************************
/// @brief  CLI command process
/// @param  cmd: command string
/// @param  argv: argument list
/// @param  argc: arguments count
/// @param  response: response
/// @retval response
/// @return true - success, false - fail
//  ***************************************************************************
bool config_cli_command_process(const char* cmd, const char (*argv)[CLI_ARG_MAX_SIZE], uint32_t argc, char* response) {

    if (strcmp(cmd, "enable") == 0) {
        sysmon_enable_module(SYSMON_MODULE_CONFIGURATOR);
    }
    else if (strcmp(cmd, "read") == 0 && argc == 1) {

        // Get page number
        uint32_t page = atoi(argv[0]);
        if (page >= CONFIG_SECTION_PAGE_COUNT) {
            strcpy(response, CLI_MSG("ERROR: Wrong page number"));
            return false;
        }

        // Read data
        if (config_read(page * CONFIG_SECTION_PAGE_SIZE, cli_buffer, CONFIG_SECTION_PAGE_SIZE) == false) {
            strcpy(response, CLI_MSG("ERROR: Cannot read data from memory"));
            return false;
        }
        
        // Formatting data
        response += sprintf(response, CLI_MSG("      00 01 02 03  04 05 06 07  08 09 0A 0B  0C 0D 0E 0F"));
        for (uint32_t i = 0; i < CONFIG_SECTION_PAGE_SIZE; i += 16) {
            response += sprintf(response, CLI_MSG("%04X: %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X"),
                                i + page * CONFIG_SECTION_PAGE_SIZE, 
                                cli_buffer[i +  0], cli_buffer[i +  1], cli_buffer[i +  2], cli_buffer[i +  3],
                                cli_buffer[i +  4], cli_buffer[i +  5], cli_buffer[i +  6], cli_buffer[i +  7],
                                cli_buffer[i +  8], cli_buffer[i +  9], cli_buffer[i + 10], cli_buffer[i + 11],
                                cli_buffer[i + 12], cli_buffer[i + 13], cli_buffer[i + 14], cli_buffer[i + 15]);
        }
    }
    else if (strcmp(cmd, "read_raw") == 0 && argc == 1) {

        // Get page number
        uint32_t page = atoi(argv[0]);
        if (page >= CONFIG_SECTION_PAGE_COUNT) {
            strcpy(response, CLI_MSG("ERROR: Wrong page number"));
            return false;
        }

        // Read data
        if (config_read(page * CONFIG_SECTION_PAGE_SIZE, cli_buffer, CONFIG_SECTION_PAGE_SIZE) == false) {
            strcpy(response, CLI_MSG("ERROR: Cannot read data from memory"));
            return false;
        }

        // Formatting data
        for (uint32_t i = 0; i < CONFIG_SECTION_PAGE_SIZE; i += 16) {
            response += sprintf(response, CLI_MSG("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"),
                                cli_buffer[i +  0], cli_buffer[i +  1], cli_buffer[i +  2], cli_buffer[i +  3],
                                cli_buffer[i +  4], cli_buffer[i +  5], cli_buffer[i +  6], cli_buffer[i +  7],
                                cli_buffer[i +  8], cli_buffer[i +  9], cli_buffer[i + 10], cli_buffer[i + 11],
                                cli_buffer[i + 12], cli_buffer[i + 13], cli_buffer[i + 14], cli_buffer[i + 15]);
        }
    }
    else if (strcmp(cmd, "read16") == 0 && argc == 2) {

        // Get address
        uint32_t address = strtol(argv[0], NULL, 16);
        if (address > CONFIG_SECTION_SIZE) {
            sprintf(response, CLI_MSG("ERROR: Wrong address value (%d)"), address);
            return false;
        }

        // Read data
        uint16_t data = 0;
        if (config_read_16(address, &data) == false) {
            strcpy(response, CLI_MSG("ERROR: Cannot read data from memory"));
            return false;
        }

        // Get sign
        if (argv[1][0] == 's') {
            sprintf(response, CLI_MSG("value = %d"), (int16_t)data);
        }
        else {
            sprintf(response, CLI_MSG("value = %d"), (uint16_t)data);
        }
    }
    else if (strcmp(cmd, "read32") == 0 && argc == 2) {

        // Get address
        uint32_t address = strtol(argv[0], NULL, 16);
        if (address > CONFIG_SECTION_SIZE) {
            sprintf(response, CLI_MSG("ERROR: Wrong address value (%d)"), address);
            return false;
        }

        // Read data
        uint32_t data = 0;
        if (config_read_32(address, &data) == false) {
            strcpy(response, CLI_MSG("Cannot read data from memory"));
            return false;
        }

        // Get sign
        if (argv[1][0] == 's') {
            sprintf(response, CLI_MSG("value = %d"), (int32_t)data);
        }
        else {
            sprintf(response, CLI_MSG("value = %u"), (uint32_t)data);
        }
    }
    else if (strcmp(cmd, "write") == 0 && argc == 2) {

        // Get address
        uint32_t address = strtol(argv[0], NULL, 16);
        if (address > CONFIG_SECTION_SIZE) {
            sprintf(response, CLI_MSG("ERROR: Wrong address value (%d)"), address);
            return false;
        }

        // Get bytes count
        uint32_t data_len = strlen(argv[1]);
        if (data_len & 0x01) {
            sprintf(response, CLI_MSG("ERROR: Wrong data array %s"), argv[1]);
            return false;
        }
        uint32_t bytes_count = data_len >> 1;

        // Parse and write data
        char hex_value[3] = {0, 0, '\0'};
        const char* data = &argv[1][0];
        for (uint32_t i = 0; i < bytes_count; ++i, ++address) {

            hex_value[0] = *(data + 0);
            hex_value[1] = *(data + 1);
            data += 2;

            uint8_t byte = strtol(hex_value, NULL, 16);
            if (config_write_8(address, byte) == false) {
                strcpy(response, CLI_MSG("ERROR: Cannot write data to memory"));
                return false;
            }
        }
    }
    else if (strcmp(cmd, "write16") == 0 && argc == 2) {

        // Get address
        uint32_t address = strtol(argv[0], NULL, 16);
        if (address > CONFIG_SECTION_SIZE) {
            sprintf(response, CLI_MSG("ERROR: Wrong address value (%d)"), address);
            return false;
        }

        // Parse data
        uint16_t data = atoi(argv[1]);

        // Write data
        if (config_write_16(address, data) == false) {
            strcpy(response, CLI_MSG("ERROR: Cannot write data to memory"));
            return false;
        }
    }
    else if (strcmp(cmd, "write32") == 0 && argc == 2) {

        // Get address
        uint32_t address = strtol(argv[0], NULL, 16);
        if (address > CONFIG_SECTION_SIZE) {
            sprintf(response, CLI_MSG("ERROR: Wrong address value (%d)"), address);
            return false;
        }

        // Parse data
        uint32_t data = atoi(argv[1]);

        // Write data
        if (config_write_32(address, data) == false) {
            strcpy(response, CLI_MSG("ERROR: Cannot write data to memory"));
            return false;
        }
    }
    else if (strcmp(cmd, "erase") == 0 && argc == 0) {

        if (config_erase() == false) {
            strcpy(response, CLI_MSG("ERROR: Cannot write data to memory"));
            return false;
        }
    }
    else if (strcmp(cmd, "verify") == 0 && argc == 1) {

        // Get page number
        uint32_t page = atoi(argv[0]);
        if (page >= CONFIG_SECTION_PAGE_COUNT) {
            return false;
        }

        if (config_verify_checksum(page) == false) {
            strcpy(response, CLI_MSG("Verify failed"));
            return false;
        }
    }
    else if (strcmp(cmd, "calc_checksum") == 0 && argc == 1) {

        // Get page number
        uint32_t page = atoi(argv[0]);
        if (page >= CONFIG_SECTION_PAGE_COUNT) {
            return false;
        }

        // Calculate checksum
        uint32_t checksum = 0;
        if (config_calc_checksum(page, &checksum) == false) {
            strcpy(response, CLI_MSG("ERROR: Cannot read data from memory"));
            return false;
        }

        // Write checksum value
        if (config_write_32(page * CONFIG_SECTION_PAGE_SIZE + MM_PAGE_CHECKSUM_OFFSET, checksum) == false) {
            strcpy(response, CLI_MSG("ERROR: Cannot write data to memory"));
            return false;
        }
    }
    else {
        sprintf(response, CLI_MSG("ERROR: Unknown command '%s' for configurator"), cmd);
        return false;
    }
    return true;
}





//  ***************************************************************************
/// @brief  Calculate checksum for page
/// @param  page: page number
/// @param  checksum: checksum value
/// @retval checksum
/// @return true - success, false - error
//  ***************************************************************************
static bool config_calc_checksum(uint32_t page, uint32_t *checksum) {

    *checksum = 0;

    uint32_t offset = page * CONFIG_SECTION_PAGE_SIZE;
    for (uint32_t address = 0; address < CONFIG_SECTION_PAGE_SIZE - 4; ++address) { // 4 - checksum size

        uint8_t byte = 0;
        if (config_read_8(address + offset, &byte) == false) {
            return false;
        }

        *checksum += byte;
    }

    return true;
}

//  ***************************************************************************
/// @brief  Verify checksum for page
/// @param  page: page number
/// @return true - success, false - error
//  ***************************************************************************
static bool config_verify_checksum(uint32_t page) {

    // Calculate checksum
    uint32_t checksum = 0;
    if (config_calc_checksum(page, &checksum) == false) {
        return false;
    }

    // Read checksum value
    uint32_t current_checksum = 0;
    if (config_read_32(page * CONFIG_SECTION_PAGE_SIZE + MM_PAGE_CHECKSUM_OFFSET, &current_checksum) == false) {
        return false;
    }

    return checksum == current_checksum;
}

//  ***************************************************************************
/// @brief  Erase EEPROM
/// @param  none
/// @return true - success, false - error
//  ***************************************************************************
static bool config_erase(void) {

    uint8_t clear_data[32] = {0};
    memset(clear_data, 0xFF, sizeof(clear_data));

    for (uint32_t address = 0; address < CONFIG_SECTION_SIZE; address += 32) {
        if (config_write(address, clear_data, sizeof(clear_data)) == false) {
            return false;
        }
    }
    return true;
}
