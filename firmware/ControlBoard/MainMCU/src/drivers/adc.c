/// ***************************************************************************
/// @file    adc.c
/// @author  NeoProg
/// ***************************************************************************
#include "adc.h"
#include "project-base.h"
#include "stm32f373xc.h"
#include "systimer.h"
#define ADC_INPUT_1_PIN                 GPIOC, 3

static bool adc_data_is_updated = false;
static uint16_t adc_data = 0;


/// ***************************************************************************
/// @brief  ADC initialization
/// ***************************************************************************
void adc_init(void) {
    gpio_set_mode        (ADC_INPUT_1_PIN, GPIO_MODE_ANALOG);
    gpio_set_output_speed(ADC_INPUT_1_PIN, GPIO_SPEED_HIGH);
    gpio_set_pull        (ADC_INPUT_1_PIN, GPIO_PULL_NO);

    // Setup ADC
    RCC->APB2RSTR |= RCC_APB2RSTR_ADC1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_ADC1RST;
    ADC1->CR1   = ADC_CR1_SCAN | ADC_CR1_EOCIE;
    ADC1->CR2   = ADC_CR2_EXTTRIG | (0x07 << ADC_CR2_EXTSEL_Pos); // SWSTART trigger
    ADC1->SMPR1 = (0x07 << ADC_SMPR1_SMP13_Pos); // 239 cycles
    ADC1->SQR3  = (13 << ADC_SQR3_SQ1_Pos); // Conversion sequence [13]
    NVIC_EnableIRQ(ADC1_IRQn);
    NVIC_SetPriority(ADC1_IRQn, ADC_IRQ_PRIORITY);
                   
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

/// ***************************************************************************
/// @brief  Start conversion
/// ***************************************************************************
void adc_start_conversion(void) {
    adc_data_is_updated = false;
    ADC1->CR2 |= ADC_CR2_SWSTART;
}

/// ***************************************************************************
/// @brief  Check conversion complete
/// @return true - success, false - in progress
/// ***************************************************************************
bool adc_is_conversion_complete(void) {
    return adc_data_is_updated;
}

/// ***************************************************************************
/// @brief  Get conversion result
/// @param  channel: channel index
/// @return conversion result
/// ***************************************************************************
uint16_t adc_read(void) {
    return adc_data;
}





/// ***************************************************************************
/// @brief  ADC1 ISR
/// ***************************************************************************
void ADC1_IRQHandler(void) {
    uint32_t status = ADC1->SR;
    if (status & ADC_SR_EOC) {
        adc_data_is_updated = true;
        adc_data = ADC1->DR;
    }
}
