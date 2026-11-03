#include "fault_manager.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define ADC_FAULT_LOW_MV    100u
#define ADC_FAULT_HIGH_MV  3200u
#define LATENCY_FAULT_MS    150u  /* > 100 ms telemetry period */

static uint8_t s_fault_mask     = 0u;
static char    s_fault_str[256] = {0};

/* Descriptor table: bit -> JSON-quoted label.
 * The output string format "LABEL_A","LABEL_B" is JSON array content;
 * the telemetry module wraps it with [...]. */
typedef struct { FaultType bit; const char *label; } FaultDesc;

static const FaultDesc k_table[] = {
    { FAULT_ADC_OUT_OF_RANGE, "\"ADC_OUT_OF_RANGE\"" },
    { FAULT_SENSOR_TIMEOUT,   "\"SENSOR_TIMEOUT\""   },
    { FAULT_BUTTON_EVENT,     "\"BUTTON_EVENT\""      },
    { FAULT_HIGH_LATENCY,     "\"HIGH_LATENCY\""      },
};

static void rebuild(void)
{
    s_fault_str[0] = '\0';
    bool first = true;
    for (size_t i = 0; i < sizeof(k_table) / sizeof(k_table[0]); i++)
    {
        if (s_fault_mask & (uint8_t)k_table[i].bit)
        {
            if (!first)
            {
                strncat(s_fault_str, ",",
                        sizeof(s_fault_str) - strlen(s_fault_str) - 1u);
            }
            strncat(s_fault_str, k_table[i].label,
                    sizeof(s_fault_str) - strlen(s_fault_str) - 1u);
            first = false;
        }
    }
}

void fault_manager_init(void)
{
    s_fault_mask    = 0u;
    s_fault_str[0]  = '\0';
}

void fault_manager_clear(void)
{
    s_fault_mask   = 0u;
    rebuild();
}

void fault_manager_update_adc(uint16_t adc_mv)
{
    if (adc_mv < ADC_FAULT_LOW_MV || adc_mv > ADC_FAULT_HIGH_MV)
        s_fault_mask |= (uint8_t)FAULT_ADC_OUT_OF_RANGE;
    else
        s_fault_mask &= (uint8_t)~(uint8_t)FAULT_ADC_OUT_OF_RANGE;
    rebuild();
}

void fault_manager_update_sensor(bool sensor_ok)
{
    if (!sensor_ok)
        s_fault_mask |= (uint8_t)FAULT_SENSOR_TIMEOUT;
    else
        s_fault_mask &= (uint8_t)~(uint8_t)FAULT_SENSOR_TIMEOUT;
    rebuild();
}

void fault_manager_update_button(bool button_pressed)
{
    if (button_pressed)
        s_fault_mask |= (uint8_t)FAULT_BUTTON_EVENT;
    else
        s_fault_mask &= (uint8_t)~(uint8_t)FAULT_BUTTON_EVENT;
    rebuild();
}

void fault_manager_update_latency(uint32_t loop_time_ms)
{
    if (loop_time_ms > LATENCY_FAULT_MS)
        s_fault_mask |= (uint8_t)FAULT_HIGH_LATENCY;
    else
        s_fault_mask &= (uint8_t)~(uint8_t)FAULT_HIGH_LATENCY;
    rebuild();
}

bool fault_manager_has_faults(void)
{
    return (s_fault_mask != 0u);
}

const char *fault_manager_get_fault_string(void)
{
    return s_fault_str;
}
