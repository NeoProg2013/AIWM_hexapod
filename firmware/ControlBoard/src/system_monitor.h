//  ***************************************************************************
/// @file    system_monitor.c
/// @author  NeoProg
/// @brief   System monitor interface
//  ***************************************************************************
#ifndef _SYSTEM_MONITOR_H_
#define _SYSTEM_MONITOR_H_

#include <stdint.h>
#include <stdbool.h>


#define SYSMON_NO_ERROR                 (0x00)
#define SYSMON_ANY_ERROR                (0xFF)                          // Not resettable (enter to emergency loop)
#define SYSMON_FATAL_ERROR              (0x01)                          // Not resettable (enter to emergency loop)
#define SYSMON_CONFIG_ERROR             (0x02 | SYSMON_FATAL_ERROR)     // Not resettable (enter to emergency loop)
#define SYSMON_MEMORY_ERROR             (0x04 | SYSMON_FATAL_ERROR)     // Not resettable (enter to emergency loop)
#define SYSMON_VOLTAGE_ERROR            (0x08)                          // Not resettable
#define SYSMON_SYNC_ERROR               (0x10)                          // Not resettable
#define SYSMON_MATH_ERROR               (0x20)                          // Not resettable
#define SYSMON_I2C_ERROR                (0x40)                          // Not resettable
#define SYSMON_CONN_LOST_ERROR          (0x80)                          // Resettable

#define SYSMON_MODULE_ALL               (0xFF)
#define SYSMON_MODULE_NO                (0x00)
#define SYSMON_MODULE_CONFIGURATOR      (0x01)
#define SYSMON_MODULE_SERVO_DRIVER      (0x02)
#define SYSMON_MODULE_MOTION_DRIVER     (0x04)
#define SYSMON_MODULE_SEQUENCES_ENGINE  (0x08)
#define SYSMON_MODULE_SYSTEM_MONITOR    (0x10)
#define SYSMON_MODULE_OLED_GL           (0x20)
#define SYSMON_MODULE_GUI               (0x40)


// NOTE: Need use external functions for access to this variables
// This variables extern for communication protocols
extern uint8_t  sysmon_system_status;
extern uint8_t  sysmon_module_status;
extern uint16_t sysmon_battery_voltage;
extern uint8_t  sysmon_battery_charge;


extern void sysmon_init(void);
extern void sysmon_process(void);

extern void sysmon_set_error(uint32_t error);
extern void sysmon_clear_error(uint32_t error);
extern void sysmon_disable_module(uint32_t module);
extern void sysmon_enable_module(uint32_t module);

extern bool sysmon_is_error_set(uint32_t error);
extern bool sysmon_is_module_disable(uint32_t module);


#endif // _SYSTEM_MONITOR_H_

