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
#define CONFIG_SECTION_PAGE_SIZE            (256u)
#define CONFIG_SECTION_PAGE_COUNT           (3u)
#define CONFIG_SECTION_SIZE                 (CONFIG_SECTION_PAGE_SIZE * CONFIG_SECTION_PAGE_COUNT)

#define CONFIG_SECTION_MAX_READ_SIZE        (128u)
#define CONFIG_SECTION_MAX_WRITE_SIZE       (32u)


static bool config_write_8(uint32_t address, uint8_t data);
static bool config_write_16(uint32_t address, uint16_t data);
static bool config_write_32(uint32_t address, uint32_t data);
static bool config_calc_checksum(uint32_t page, uint16_t *checksum);
static bool config_check_page_integrity(uint32_t page);
static bool config_update_checksum(uint32_t page);

CLI_CMD_HANDLER(config_cli_cmd_help);
CLI_CMD_HANDLER(config_cli_cmd_enable);
CLI_CMD_HANDLER(config_cli_cmd_read_page);
CLI_CMD_HANDLER(config_cli_cmd_read);
CLI_CMD_HANDLER(config_cli_cmd_write);
CLI_CMD_HANDLER(config_cli_cmd_check);
CLI_CMD_HANDLER(config_cli_cmd_update);

static const cli_cmd_t cli_cmd_list[] = {
    { .cmd = "help",      .handler = config_cli_cmd_help      },
    { .cmd = "enable",    .handler = config_cli_cmd_enable    },
    { .cmd = "read-page", .handler = config_cli_cmd_read_page },
    { .cmd = "read",      .handler = config_cli_cmd_read      },
    { .cmd = "write",     .handler = config_cli_cmd_write     },
    { .cmd = "check",     .handler = config_cli_cmd_check     },
    { .cmd = "update",    .handler = config_cli_cmd_update    }
};


//  ***************************************************************************
/// @brief  Configurator initialization
/// @param  none
/// @return none
//  ***************************************************************************
void config_init(void) {
    i2c1_init(I2C_SPEED_400KHZ);
}

//  ***************************************************************************
/// @brief  Check EEPROM intergity
/// @param  none
/// @return true - success, false - otherwise
//  ***************************************************************************
bool config_check_intergity(void) { 
    for (uint32_t page = 0; page < CONFIG_SECTION_PAGE_COUNT; ++page) {
        if (config_check_page_integrity(page) == false) {
            sysmon_set_error(SYSMON_MEMORY_ERROR);
            sysmon_disable_module(SYSMON_MODULE_CONFIGURATOR);
            return false;
        }
    }
    
    // Check memory map version
    uint16_t version = 0;
    if (!config_read_16(MM_VERSION_EE_ADDRESS, &version)) {
        sysmon_set_error(SYSMON_MEMORY_ERROR);
        sysmon_disable_module(SYSMON_MODULE_CONFIGURATOR);
        return false;
    }
    if (version != MM_VERSION) {
        sysmon_set_error(SYSMON_MEMORY_ERROR);
        sysmon_disable_module(SYSMON_MODULE_CONFIGURATOR);
        return false;
    }
    return true;
}

//  ***************************************************************************
/// @brief  Read data from configuration section
/// @param  address: data block address
/// @param  buffer: buffer for data
/// @param  bytes_count: bytes count for read
/// @return true - success, false - error
//  ***************************************************************************
bool config_read(uint32_t address, uint8_t* buffer, uint32_t bytes_count) {
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
            sysmon_set_error(SYSMON_COMM_ERROR | SYSMON_MEMORY_ERROR);
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
            sysmon_set_error(SYSMON_COMM_ERROR | SYSMON_MEMORY_ERROR);
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
/// @brief  Get command list for CLI
/// @param  cmd_list: pointer to cmd list size
/// @return command list
//  ***************************************************************************
const cli_cmd_t* config_get_cmd_list(uint32_t* count) {
    *count = sizeof(cli_cmd_list) / sizeof(cli_cmd_t);
    return cli_cmd_list;
}


//  ***************************************************************************
/// @brief  Write BYTE/WORD/DWORD to configuration section
/// @param  address: data block address
/// @param  data: data for write
/// @return true - success, false - error
//  ***************************************************************************
static bool config_write_8(uint32_t address, uint8_t data)   { return config_write(address, (uint8_t*)&data, 1); }
static bool config_write_16(uint32_t address, uint16_t data) { return config_write(address, (uint8_t*)&data, 2); }
static bool config_write_32(uint32_t address, uint32_t data) { return config_write(address, (uint8_t*)&data, 4); }

//  ***************************************************************************
/// @brief  Calculate checksum for page
/// @param  page: page number
/// @param  checksum: checksum value
/// @retval checksum
/// @return true - success, false - error
//  ***************************************************************************
static bool config_calc_checksum(uint32_t page, uint16_t *checksum) {
    *checksum = 0;

    uint32_t offset = page * CONFIG_SECTION_PAGE_SIZE;
    for (uint32_t address = 0; address < CONFIG_SECTION_PAGE_SIZE - MM_PAGE_CHECKSUM_SIZE; ++address) {
        uint8_t byte = 0;
        if (config_read_8(address + offset, &byte) == false) {
            return false;
        }
        *checksum += byte;
    }
    return true;
}

//  ***************************************************************************
/// @brief  Check page integrity
/// @param  page: page number
/// @return true - success, false - error
//  ***************************************************************************
static bool config_check_page_integrity(uint32_t page) {
    uint16_t checksum = 0;
    if (config_calc_checksum(page, &checksum) == false) {
        return false;
    }

    // Read checksum value
    uint16_t current_checksum = 0;
    if (config_read_16(page * CONFIG_SECTION_PAGE_SIZE + MM_PAGE_CHECKSUM_OFFSET, &current_checksum) == false) {
        return false;
    }
    return checksum == current_checksum;
}

//  ***************************************************************************
/// @brief  Update checksum for page
/// @param  page: page number
/// @return true - success, false - error
//  ***************************************************************************
static bool config_update_checksum(uint32_t page) {
    uint16_t checksum = 0;
    if (config_calc_checksum(page, &checksum) == false) {
        return false;
    }
    return config_write_16(page * CONFIG_SECTION_PAGE_SIZE + MM_PAGE_CHECKSUM_OFFSET, checksum);
}





// ***************************************************************************
// CLI SECTION
// ***************************************************************************
static uint8_t cli_buffer[CONFIG_SECTION_PAGE_SIZE] = {0};

CLI_CMD_HANDLER(config_cli_cmd_help) {
    const char* help = CLI_HELP(
        "[CONFIG SUBSYSTEM]\r\n"
        "Commands: \r\n"
        "  config enable - enable subsystem after error\r\n"
        "  config read-page <page idx> [raw] - read page data\r\n"
        "  config read <hex address> <8|16|32> - read cell value\r\n"
        "  config write <hex address> <8|16|32|hex array> <value>\r\n"
        "  config check <page idx> - check page integrity\r\n"
        "  config update <page idx> - update page checksum");
    if (strlen(response) >= USART1_TX_BUFFER_SIZE) {
        strcpy(response, CLI_ERROR("Help message size more USART1_TX_BUFFER_SIZE"));
        return false;
    }
    strcpy(response, help);
    return true;
}
CLI_CMD_HANDLER(config_cli_cmd_enable) {
    sysmon_enable_module(SYSMON_MODULE_CONFIGURATOR);
    return true;
}
CLI_CMD_HANDLER(config_cli_cmd_read_page) {
    if (argc < 1) {
        strcpy(response, CLI_ERROR("Bad usage. Use \"config help\" for details"));
        return false;
    }
    
    // Read data: argv[0] - page number
    uint32_t page = atoi(argv[0]);
    if (config_read(page * CONFIG_SECTION_PAGE_SIZE, cli_buffer, CONFIG_SECTION_PAGE_SIZE) == false) {
        sprintf(response, CLI_ERROR("Can't read data by address 0x%04X"), page * CONFIG_SECTION_PAGE_SIZE);
        return false;
    }

    // Formatting data
    if (argc == 1) {
        response += sprintf(response, CLI_MSG(CLI_COLOR_CYAN "      00 01 02 03  04 05 06 07  08 09 0A 0B  0C 0D 0E 0F"));
        for (uint32_t i = 0; i < CONFIG_SECTION_PAGE_SIZE; i += 16) {
            response += sprintf(response, CLI_MSG(CLI_COLOR_CYAN "%04X: " CLI_COLOR_GREEN "%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X"),
                                i + page * CONFIG_SECTION_PAGE_SIZE, 
                                cli_buffer[i +  0], cli_buffer[i +  1], cli_buffer[i +  2], cli_buffer[i +  3],
                                cli_buffer[i +  4], cli_buffer[i +  5], cli_buffer[i +  6], cli_buffer[i +  7],
                                cli_buffer[i +  8], cli_buffer[i +  9], cli_buffer[i + 10], cli_buffer[i + 11],
                                cli_buffer[i + 12], cli_buffer[i + 13], cli_buffer[i + 14], cli_buffer[i + 15]);
        }
    } else if (strcmp(argv[1], "raw") == 0) {
        for (uint32_t i = 0; i < CONFIG_SECTION_PAGE_SIZE; i += 16) {
            response += sprintf(response, CLI_OK("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"),
                                cli_buffer[i +  0], cli_buffer[i +  1], cli_buffer[i +  2], cli_buffer[i +  3],
                                cli_buffer[i +  4], cli_buffer[i +  5], cli_buffer[i +  6], cli_buffer[i +  7],
                                cli_buffer[i +  8], cli_buffer[i +  9], cli_buffer[i + 10], cli_buffer[i + 11],
                                cli_buffer[i + 12], cli_buffer[i + 13], cli_buffer[i + 14], cli_buffer[i + 15]);
        }
    }
    return true;
}
CLI_CMD_HANDLER(config_cli_cmd_read) {
    if (argc < 2) {
        strcpy(response, CLI_ERROR("Bad usage. Use \"config help\" for details"));
        return false;
    }
    
    uint32_t address = strtol(argv[0], NULL, 16);
    if (strcmp(argv[1], "8") == 0) {
        uint8_t data = 0;
        if (!config_read_8(address, &data)) {
            sprintf(response, CLI_ERROR("Can't read data by address 0x%04X"), address);
            return false;
        }
        sprintf(response, CLI_OK("value = %d(s) %u(u)"), (int8_t)data, (uint8_t)data);
    }
    else if (strcmp(argv[1], "16") == 0) {
        uint16_t data = 0;
        if (!config_read_16(address, &data)) {
            sprintf(response, CLI_ERROR("Can't read data by address 0x%04X"), address);
            return false;
        }
        sprintf(response, CLI_OK("value = %d(s) %u(u)"), (int16_t)data, (uint16_t)data);
    } 
    else if (strcmp(argv[1], "32") == 0) {
        uint32_t data = 0;
        if (!config_read_32(address, &data)) {
            sprintf(response, CLI_ERROR("Can't read data by address 0x%04X"), address);
            return false;
        }
        sprintf(response, CLI_OK("value = %d(s) %u(u)"), (int32_t)data, (uint32_t)data);
    } 
    return true;
}
CLI_CMD_HANDLER(config_cli_cmd_write) {
    if (argc < 1) {
        strcpy(response, CLI_ERROR("Bad usage. Use \"config help\" for details"));
        return false;
    }
    
    uint32_t address = strtol(argv[0], NULL, 16); // Get address
    bool result = true;
    if (argc >= 3 && strcmp(argv[1], "8") == 0) {
        result = config_write_8(address, (uint8_t)atoi(argv[2]));
    } 
    else if (argc >= 3 && strcmp(argv[1], "16") == 0) {
        result = config_write_16(address, (uint16_t)atoi(argv[2]));
    } 
    else if (argc >= 3 && strcmp(argv[1], "32") == 0) {
        result = config_write_32(address, (uint32_t)atoi(argv[2]));
    } 
    else {
        uint32_t data_len = strlen(argv[1]); // Get bytes count
        if (data_len & 0x01) {
            strcpy(response, CLI_ERROR("Wrong data array - bytes count value should be even"));
            return false;
        }
        uint32_t bytes_count = data_len >> 1;

        // Parse and write data
        char hex_value[3] = {0, 0, '\0'};
        const char* data = &argv[1][0];
        for (uint32_t i = 0; i < bytes_count && result; ++i, ++address) {
            hex_value[0] = *(data + 0);
            hex_value[1] = *(data + 1);
            data += 2;
            result = config_write_8(address, strtol(hex_value, NULL, 16));
        }
    }
    
    if (!result) {
        sprintf(response, CLI_ERROR("Can't write data by address 0x%04X"), address);
    }
    return result;
}
CLI_CMD_HANDLER(config_cli_cmd_check) {
    if (argc != 1) {
        strcpy(response, CLI_ERROR("Bad usage. Use \"config help\" for details"));
        return false;
    }
    
    uint32_t page = atoi(argv[0]);
    if (page >= CONFIG_SECTION_PAGE_COUNT) {
        sprintf(response, CLI_ERROR("Page number (%u) is out of range, it should be less %u"), page, CONFIG_SECTION_PAGE_COUNT);
        return false;
    }

    if (config_check_page_integrity(page) == false) {
        strcpy(response, CLI_ERROR("Page integrity is broken!"));
        return false;
    }
    return true;
}
CLI_CMD_HANDLER(config_cli_cmd_update) {
    if (argc != 1) {
        strcpy(response, CLI_ERROR("Bad usage. Use \"config help\" for details"));
        return false;
    }
    
    uint32_t page = atoi(argv[0]);
    if (page >= CONFIG_SECTION_PAGE_COUNT) {
        sprintf(response, CLI_ERROR("Page number (%u) is out of range, it should be less %d"), page, CONFIG_SECTION_PAGE_COUNT);
        return false;
    }

    // Update checksum
    if (config_update_checksum(page) == false) {
        strcpy(response, CLI_ERROR("Can't update checksum value"));
        return false;
    }
    return true;
}