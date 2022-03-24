/// ***************************************************************************
/// @file    sensors-core.h
/// @author  NeoProg
/// @brief   Sensors core
/// ***************************************************************************
#ifndef _SENSORS_CORE_H_
#define _SENSORS_CORE_H_

extern void sensors_core_init(void); 
extern bool sensors_core_calibration_process(void);
extern void sensors_core_process(void);

#endif // _SENSORS_CORE_H_
