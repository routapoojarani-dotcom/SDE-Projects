#ifndef FAULT_MANAGER_H
#define FAULT_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    FAULT_NONE             = 0x00u,
    FAULT_ADC_OUT_OF_RANGE = 0x01u,
    FAULT_SENSOR_TIMEOUT   = 0x02u,
    FAULT_BUTTON_EVENT     = 0x04u,
    FAULT_HIGH_LATENCY     = 0x08u,
} FaultType;

void        fault_manager_init(void);
void        fault_manager_clear(void);
void        fault_manager_update_adc(uint16_t adc_mv);
void        fault_manager_update_sensor(bool sensor_ok);
void        fault_manager_update_button(bool button_pressed);
void        fault_manager_update_latency(uint32_t loop_time_ms);
bool        fault_manager_has_faults(void);
const char *fault_manager_get_fault_string(void);

#endif /* FAULT_MANAGER_H */
