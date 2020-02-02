//  ***************************************************************************
/// @file    adc.c
/// @author  NeoProg
//  ***************************************************************************
#include "adc.h"
#include "stm32f373xc.h"
#include "systimer.h"

#define ADC_CHANNELS_COUNT              (3)
#define ADC_INPUT_1_PIN                 (3) // PC3 ADC CH13
#define ADC_INPUT_2_PIN                 (2) // PC2 ADC CH12
#define ADC_INPUT_3_PIN                 (1) // PC1 ADC CH11


static uint16_t adc_data[ADC_CHANNELS_COUNT] = {0};


//  ***************************************************************************
/// @brief  ADC initialization
/// @param  none
/// @return none
//  ***************************************************************************
void adc_init(void) {
    
    //
    // Setup GPIO
    //
    // Setup analog input 1 (PC3)
    GPIOC->MODER   |=  (0x03 << (ADC_INPUT_1_PIN * 2));         // Analog mode
    GPIOC->OSPEEDR |=  (0x03 << (ADC_INPUT_1_PIN * 2));         // High speed
    GPIOC->PUPDR   &= ~(0x03 << (ADC_INPUT_1_PIN * 2));         // Disable pull
    
    // Setup analog input 2 (PC2)
    GPIOC->MODER   |=  (0x03 << (ADC_INPUT_2_PIN * 2));         // Analog mode
    GPIOC->OSPEEDR |=  (0x03 << (ADC_INPUT_2_PIN * 2));         // High speed
    GPIOC->PUPDR   &= ~(0x03 << (ADC_INPUT_2_PIN * 2));         // Disable pull
    
    // Setup analog input 3 (PC1)
    GPIOC->MODER   |=  (0x03 << (ADC_INPUT_3_PIN * 2));         // Analog mode
    GPIOC->OSPEEDR |=  (0x03 << (ADC_INPUT_3_PIN * 2));         // High speed
    GPIOC->PUPDR   &= ~(0x03 << (ADC_INPUT_3_PIN * 2));         // Disable pull


    //
    // Setup DMA
    //
    DMA1_Channel1->CCR  &= ~DMA_CCR_EN;
    DMA1_Channel1->CCR   = (0x01 << DMA_CCR_MSIZE_Pos) | (0x01 << DMA_CCR_PSIZE_Pos) | DMA_CCR_MINC | DMA_CCR_CIRC;
    DMA1_Channel1->CPAR  = (uint32_t)(&ADC1->DR);
    DMA1_Channel1->CMAR  = (uint32_t)adc_data;
    DMA1_Channel1->CNDTR = ADC_CHANNELS_COUNT;
    DMA1_Channel1->CCR  |= DMA_CCR_EN;

    
    //
    // Setup ADC
    //
    RCC->APB2RSTR |= RCC_APB2RSTR_ADC1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_ADC1RST;

    // Setup ADC
    ADC1->CR1   = ADC_CR1_SCAN;
    ADC1->CR2   = ADC_CR2_EXTTRIG | (0x07 << ADC_CR2_EXTSEL_Pos) | ADC_CR2_DMA; // SWSTART trigger
    ADC1->SMPR1 = (0x01 << ADC_SMPR1_SMP13_Pos) | (0x01 << ADC_SMPR1_SMP12_Pos) | (0x01 << ADC_SMPR1_SMP11_Pos); // 41.5 cycles
    ADC1->SQR1  = (ADC_CHANNELS_COUNT - 1) << ADC_SQR1_L_Pos; // 3 channels count for conversion
    ADC1->SQR3  = (13 << ADC_SQR3_SQ3_Pos) | (12 << ADC_SQR3_SQ2_Pos) | (11 << ADC_SQR3_SQ1_Pos); // Conversion sequence [11][12][13]

    // Enable ADC
    ADC1->CR2 |= ADC_CR2_ADON;
    while ((ADC1->CR2 & ADC_CR2_ADON) == 0);
    
    // Delay for ADC stabilization time
    delay_ms(10);

    // Reset calibration registers
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while (ADC1->CR2 & ADC_CR2_RSTCAL);

    // Calibration
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL);

    // Make 10 first ADC conversions
    for (uint32_t i = 0; i < 10; ++i) {
        adc_start_conversion();
        while (adc_is_conversion_complete() == false);
    }
}

//  ***************************************************************************
/// @brief  Start conversion
/// @param  none
/// @return none
//  ***************************************************************************
void adc_start_conversion(void) {
    DMA1->IFCR = DMA_IFCR_CGIF1;
    ADC1->CR2 |= ADC_CR2_SWSTART;
}

//  ***************************************************************************
/// @brief  Check conversion complete
/// @param  none
/// @return true - success, false - in progress
//  ***************************************************************************
bool adc_is_conversion_complete(void) {
    return DMA1->ISR & DMA_ISR_TCIF1;
}

//  ***************************************************************************
/// @brief  Get conversion result
/// @param  channel: channel index
/// @return conversion result
//  ***************************************************************************
uint16_t adc_get_conversion_result(uint32_t channel) {
    
    if (channel > ADC_CHANNELS_COUNT) {
        return 0;
    }
    return adc_data[channel];
}
