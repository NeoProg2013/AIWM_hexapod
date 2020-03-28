//  ***************************************************************************
/// @file    system_monitor.c
/// @author  NeoProg
/// @brief   System monitor interface
//  ***************************************************************************
#ifndef _SYSTEM_MONITOR_H_
#define _SYSTEM_MONITOR_H_

#include <stdint.h>
#include <stdbool.h>


typedef enum {

    // General
    SYSMON_NO_ERROR                 = 0x00,
    SYSMON_ANY_ERROR                = 0xFF,
    SYSMON_FATAL_ERROR              = 0x01,                          // Not resettable (enter to emergency loop)

    // User defined errors
    SYSMON_CONFIG_ERROR             = 0x02 | SYSMON_FATAL_ERROR,     // Not resettable (enter to emergency loop)
    SYSMON_MEMORY_ERROR             = 0x04 | SYSMON_FATAL_ERROR,     // Not resettable (enter to emergency loop)
    SYSMON_VOLTAGE_ERROR            = 0x08,                          // Not resettable
    SYSMON_SYNC_ERROR               = 0x10,                          // Not resettable
    SYSMON_MATH_ERROR               = 0x20,                          // Not resettable
    // RESERVED
    SYSMON_CONN_LOST_ERROR          = 0x80                           // Resettable

} sysmon_error_t;

typedef enum {

    // General
    SYSMON_MODULE_ALL               = 0xFF,
    SYSMON_MODULE_NO                = 0x00,

    // Users defined modules
    SYSMON_MODULE_CONFIGURATOR      = 0x01,
    SYSMON_MODULE_SERVO_DRIVER      = 0x02,
    SYSMON_MODULE_MOTION_DRIVER     = 0x04,
    SYSMON_MODULE_SEQUENCES_ENGINE  = 0x08,
    SYSMON_MODULE_SYSTEM_MONITOR    = 0x10

} sysmon_module_t;


// NOTE: Need use external functions for access to this variables
// This variables extern for communication protocols
extern uint8_t  sysmon_system_status;
extern uint8_t  sysmon_module_status;
extern uint16_t sysmon_battery_cell_voltage[3];
extern uint16_t sysmon_battery_voltage;
extern uint8_t  sysmon_battery_charge;


extern void sysmon_init(void);
extern void sysmon_process(void);

extern void sysmon_set_error(sysmon_error_t error);
extern void sysmon_clear_error(sysmon_error_t error);
extern void sysmon_disable_module(sysmon_module_t module);
extern void sysmon_enable_module(sysmon_module_t module);

extern bool sysmon_is_error_set(sysmon_error_t error);
extern bool sysmon_is_module_disable(sysmon_module_t module);


#endif // _SYSTEM_MONITOR_H_

