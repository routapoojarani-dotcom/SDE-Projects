#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>
#include <stdbool.h>

#define TELEMETRY_BUFFER_SIZE 512

void     telemetry_init(void);
uint32_t telemetry_next_seq(void);

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
    const char *fault_list
);

#endif /* TELEMETRY_H */
