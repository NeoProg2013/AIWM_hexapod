//  ***************************************************************************
/// @file    adc.h
/// @author  NeoProg
/// @brief   Interface for ADC driver
//  ***************************************************************************
#ifndef _ADC_H_
#define _ADC_H_
#include <stdint.h>
#include <stdbool.h>


extern void adc_init(void);
extern void adc_start_conversion(void);
extern bool adc_is_conversion_complete(void);
extern uint16_t adc_read(void);


#endif // _ADC_H_

