/// ***************************************************************************
/// @file    systimer.c
/// @author  NeoProg
/// ***************************************************************************
#include "systimer.h"
#include "stm32f373xc.h"
#include "project-base.h"


static volatile uint64_t systime_ms = 0;


/// ***************************************************************************
/// @brief  System timer initialize
/// @param  none
/// @return none
/// ***************************************************************************
void systimer_init(void) {
    
    systime_ms = 0;
    
    // Systimer setup 
    SysTick->VAL = 0;
    SysTick->LOAD = SYSTEM_CLOCK_FREQUENCY / 1000;
    SysTick->CTRL = (1 << SysTick_CTRL_TICKINT_Pos) | (1 << SysTick_CTRL_CLKSOURCE_Pos) | (1 << SysTick_CTRL_ENABLE_Pos);
    
    // Enable systimer
    SysTick->CTRL |= (1 << SysTick_CTRL_ENABLE_Pos);
    NVIC_EnableIRQ(SysTick_IRQn);
}

/// ***************************************************************************
/// @brief  Get current time in milliseconds
/// @param  none
/// @return Milliseconds
/// ***************************************************************************
uint64_t get_time_ms(void) {
    return systime_ms;
}

/// ***************************************************************************
/// @brief  Synchronous delay
/// @param  ms: time delay [ms]
/// @return none
/// ***************************************************************************
void delay_ms(uint32_t ms) {
    uint64_t start = systime_ms;
    while (systime_ms - start < ms);
}





/// ***************************************************************************
/// @brief  Systimer ISR
/// @return none
/// ***************************************************************************
#pragma call_graph_root="interrupt"
void SysTick_Handler(void) {
    ++systime_ms;
}
