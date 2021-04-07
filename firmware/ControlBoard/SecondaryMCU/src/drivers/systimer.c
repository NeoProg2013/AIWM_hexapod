//  ***************************************************************************
/// @file    systimer.c
/// @author  NeoProg
//  ***************************************************************************
#include "systimer.h"
#include "stm32f0xx.h"
#include "project_base.h"


static volatile uint64_t systime_ms = 0;
static uint64_t step = 1;


//  ***************************************************************************
/// @brief  System timer initialize
/// @param  none
/// @return none
//  ***************************************************************************
void systimer_init(void) {
    
    systime_ms = 0;
    step = 1;
    
    // Systimer setup 
    SysTick->VAL = 0;
    SysTick->LOAD = SYSTEM_CLOCK_FREQUENCY / 1000; // One tick = 1ms
    SysTick->CTRL = (1 << SysTick_CTRL_TICKINT_Pos) | (1 << SysTick_CTRL_CLKSOURCE_Pos) | (1 << SysTick_CTRL_ENABLE_Pos);
    
    // Enable systimer
    SysTick->CTRL |= (1 << SysTick_CTRL_ENABLE_Pos);
    NVIC_EnableIRQ(SysTick_IRQn);
}

//  ***************************************************************************
/// @brief  System timer change step
/// @param  step_ms: step
/// @return none
//  ***************************************************************************
void systimer_change_step(uint32_t step_ms) {
    SysTick->CTRL &= ~(1 << SysTick_CTRL_ENABLE_Pos);
    SysTick->LOAD = SYSTEM_CLOCK_FREQUENCY / (1000 / step_ms);
    step = step_ms;
    SysTick->CTRL |= (1 << SysTick_CTRL_ENABLE_Pos);
}

//  ***************************************************************************
/// @brief  Get current time in milliseconds
/// @param  none
/// @return Milliseconds
//  ***************************************************************************
uint64_t get_time_ms(void) {
    return systime_ms;
}

//  ***************************************************************************
/// @brief  Synchronous delay
/// @param  ms: time delay [ms]
/// @return none
//  ***************************************************************************
void delay_ms(uint32_t ms) {
    uint64_t start = systime_ms;
    while (systime_ms - start < ms);
}





//  ***************************************************************************
/// @brief  Systimer ISR
/// @return none
//  ***************************************************************************
void SysTick_Handler(void) {
    systime_ms += step;
}
