#ifndef GPIO_STATUS_H
#define GPIO_STATUS_H

#include <stdbool.h>

void gpio_status_init(void);
bool button_is_pressed(void);
void led_set_healthy(bool enabled);
void led_set_fault(bool enabled);

#endif /* GPIO_STATUS_H */
