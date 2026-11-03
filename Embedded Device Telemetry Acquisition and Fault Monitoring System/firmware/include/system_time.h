#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include <stdint.h>

void     system_time_init(void);
uint32_t system_time_ms(void);
void     system_time_delay_ms(uint32_t ms);

#endif /* SYSTEM_TIME_H */
