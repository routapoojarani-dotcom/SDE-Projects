#include "system_time.h"
#include <stdint.h>

/*
 * Simulated tick counter.
 * On real STM32 hardware: read HAL_GetTick() which returns SysTick ms count.
 * system_time_delay_ms() would call HAL_Delay().
 */
static uint32_t s_tick_ms = 0u;

void system_time_init(void)
{
    s_tick_ms = 0u;
}

uint32_t system_time_ms(void)
{
    return s_tick_ms;
}

void system_time_delay_ms(uint32_t ms)
{
    /* Simulation: advance the simulated clock.
     * Real hardware: HAL_Delay(ms) or SysTick-based busy-wait. */
    s_tick_ms += ms;
}
