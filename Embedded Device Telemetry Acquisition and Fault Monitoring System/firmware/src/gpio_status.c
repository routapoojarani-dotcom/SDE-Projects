#include "gpio_status.h"
#include "system_time.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Simulated GPIO for LEDs and push-button.
 * Real STM32: configure GPIO pins with HAL_GPIO_Init(),
 * use HAL_GPIO_WritePin() for LEDs and HAL_GPIO_ReadPin() for button.
 *
 * Green LED -> PA5 (LD2 on Nucleo-F401RE).
 * Red LED   -> PB3.
 * Button    -> PC13 (B1 USER on Nucleo), active-low with internal pull-up.
 */
static bool s_led_healthy = false;
static bool s_led_fault   = false;

void gpio_status_init(void)
{
    s_led_healthy = false;
    s_led_fault   = false;
}

bool button_is_pressed(void)
{
    /* Simulate: button held briefly once every 5 seconds (500 ms window).
     * Real: return (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET). */
    uint32_t t = system_time_ms();
    return ((t % 5000u) < 500u);
}

void led_set_healthy(bool enabled)
{
    if (enabled != s_led_healthy)
    {
        s_led_healthy = enabled;
        /* Real: HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,
         *           enabled ? GPIO_PIN_SET : GPIO_PIN_RESET); */
    }
}

void led_set_fault(bool enabled)
{
    if (enabled != s_led_fault)
    {
        s_led_fault = enabled;
        /* Real: HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3,
         *           enabled ? GPIO_PIN_SET : GPIO_PIN_RESET); */
    }
}
