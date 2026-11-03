#include "telemetry.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static uint32_t s_seq = 0u;

void telemetry_init(void)
{
    s_seq = 0u;
}

uint32_t telemetry_next_seq(void)
{
    return ++s_seq;
}

/*
 * fault_list is the content returned by fault_manager_get_fault_string():
 *   ""                                   -> faults:[]
 *   "\"ADC_OUT_OF_RANGE\""               -> faults:["ADC_OUT_OF_RANGE"]
 *   "\"ADC_OUT_OF_RANGE\",\"BUTTON_EVENT\"" -> faults:["ADC_OUT_OF_RANGE","BUTTON_EVENT"]
 *
 * The fault_manager already provides JSON-quoted labels; this function
 * wraps them in [...] to form a valid JSON array.
 */
int telemetry_build_packet(
    char       *buffer,
    int         buffer_size,
    uint32_t    seq,
    uint32_t    time_ms,
    uint16_t    adc_mv,
    float       sensor_x,
    float       sensor_y,
    float       sensor_z,
    bool        button_pressed,
    bool        has_faults,
    const char *fault_list)
{
    if (!buffer || buffer_size < 64)
    {
        return -1;
    }

    const char *status  = has_faults ? "FAULT" : "OK";
    const char *faults  = (fault_list && fault_list[0] != '\0') ? fault_list : "";

    int n = snprintf(buffer, (size_t)buffer_size,
        "{\"type\":\"telemetry\","
        "\"seq\":%lu,"
        "\"time_ms\":%lu,"
        "\"adc_mv\":%u,"
        "\"sensor_x\":%.2f,"
        "\"sensor_y\":%.2f,"
        "\"sensor_z\":%.2f,"
        "\"button\":%d,"
        "\"status\":\"%s\","
        "\"faults\":[%s]}",
        (unsigned long)seq,
        (unsigned long)time_ms,
        (unsigned int)adc_mv,
        (double)sensor_x,
        (double)sensor_y,
        (double)sensor_z,
        button_pressed ? 1 : 0,
        status,
        faults);

    if (n < 0 || n >= buffer_size)
    {
        return -1;
    }

    return n;
}
