/// ***************************************************************************
/// @file    smcu.h
/// @author  NeoProg
/// @brief   SMCU communication driver
/// ***************************************************************************
#ifndef _SMCU_H_
#define _SMCU_H_
#include <stdint.h>


extern void smcu_init(void);
extern void smcu_process(void);
extern void smcu_get_sensor_data(int16_t** foot_sensors, int32_t** accel_sensor);


#endif // _SMCU_H_
