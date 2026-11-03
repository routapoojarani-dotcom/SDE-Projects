#include "sensor_i2c.h"
#include "sensor_adc.h"
#include <stdbool.h>
#include <stdint.h>

/*
 * Simulated MPU-6050 3-axis accelerometer over I2C.
 * Real STM32: send I2C START, write device address (0x68) + register (0x3B),
 * read 6 bytes of big-endian int16 (ax, ay, az), divide by 16384.0f for ±2g.
 * Check WHO_AM_I register (0x75) == 0x68 during init.
 *
 * Simulation: near-static vertical orientation with small noise.
 * Injects a read failure every 50 reads to exercise SENSOR_TIMEOUT fault.
 */
static uint8_t s_fail_counter = 0u;

void i2c_sensor_init(void)
{
    s_fail_counter = 0u;
}

bool i2c_sensor_read(float *x, float *y, float *z)
{
    if (!x || !y || !z)
    {
        return false;
    }

    s_fail_counter++;
    if (s_fail_counter >= 50u)
    {
        s_fail_counter = 0u;
        return false; /* Simulate I2C NAK / timeout */
    }

    /* Reuse ADC raw value as a cheap noise source (avoids rand() / math.h) */
    uint16_t noise_raw = adc_read_raw();
    float    noise     = ((float)noise_raw - 2047.5f) / 2047.5f * 0.05f;

    *x =  0.02f + noise * 0.3f;
    *y = -0.01f + noise * 0.2f;
    *z =  0.98f + noise * 0.1f;

    return true;
}
