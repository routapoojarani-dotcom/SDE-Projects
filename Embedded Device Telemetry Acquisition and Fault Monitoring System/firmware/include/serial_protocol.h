#ifndef SERIAL_PROTOCOL_H
#define SERIAL_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#define DEFAULT_PERIOD_MS 100u

void     serial_protocol_init(void);
void     serial_protocol_handle_line(const char *line);
bool     serial_protocol_streaming_enabled(void);
uint32_t serial_protocol_get_period_ms(void);

#endif /* SERIAL_PROTOCOL_H */
