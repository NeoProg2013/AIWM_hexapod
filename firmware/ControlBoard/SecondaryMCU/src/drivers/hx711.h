//  ***************************************************************************
/// @file    hx711.h
/// @author  NeoProg
/// @brief   x6 HX711 parallel driver 
//  ***************************************************************************
#ifndef _HX711_H_
#define _HX711_H_

#include <stdint.h>
#include <stdbool.h>


extern void hx711_init(void);
extern void hx711_power_up(void);
extern void hx711_power_down(void);
extern bool hx711_calibration(void);
extern bool hx711_process(void);
extern bool hx711_read(int32_t* data);


#endif // _HX711_H_
