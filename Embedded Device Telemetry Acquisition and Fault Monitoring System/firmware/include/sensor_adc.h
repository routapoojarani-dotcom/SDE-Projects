#ifndef SENSOR_ADC_H
#define SENSOR_ADC_H

#include <stdint.h>

#define ADC_RESOLUTION  4095u
#define ADC_VREF_MV     3300u

void     adc_init(void);
uint16_t adc_read_raw(void);
uint16_t adc_raw_to_mv(uint16_t raw);

#endif /* SENSOR_ADC_H */
