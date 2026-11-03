#include "sensor_adc.h"
#include <stdint.h>

/*
 * Simulated 12-bit ADC on PA0 (potentiometer).
 * Real STM32: configure ADC1 with DMA, calibrate, trigger conversions,
 * read HAL_ADC_GetValue() after HAL_ADC_PollForConversion().
 *
 * Simulation uses a linear-congruential walk to produce a slowly drifting
 * value across the 12-bit range without pulling in <math.h>.
 */
static uint16_t s_raw = 2048u;

void adc_init(void)
{
    s_raw = 2048u;
}

uint16_t adc_read_raw(void)
{
    /* LCG: produces values spread across 0..4095 */
    s_raw = (uint16_t)(((uint32_t)s_raw * 6364u + 1013u) & 0x0FFFu);
    return s_raw;
}

uint16_t adc_raw_to_mv(uint16_t raw)
{
    /* 12-bit ADC, 3.3 V reference.
     * Cast to uint32_t before multiply to prevent 16-bit overflow:
     * max: 4095 * 3300 = 13,513,500 which fits in uint32_t. */
    return (uint16_t)((uint32_t)raw * ADC_VREF_MV / ADC_RESOLUTION);
}
