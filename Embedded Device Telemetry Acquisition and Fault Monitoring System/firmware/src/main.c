#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "system_time.h"
#include "sensor_adc.h"
#include "sensor_i2c.h"
#include "gpio_status.h"
#include "fault_manager.h"
#include "telemetry.h"
#include "serial_protocol.h"

/* ------------------------------------------------------------------ */
/* Simulated host command sequence (replays once then goes silent).   */
/* On real STM32: UART RX interrupt fills a ring buffer; main() drains */
/* complete lines and calls serial_protocol_handle_line().            */
/* ------------------------------------------------------------------ */
static const char *const k_sim_commands[] = {
    "PING",
    "START",
    "GET_STATUS",
    "SET_RATE 50",
    NULL,
};
static int s_cmd_index = 0;

static void poll_serial(void)
{
    if (k_sim_commands[s_cmd_index] != NULL)
    {
        serial_protocol_handle_line(k_sim_commands[s_cmd_index]);
        s_cmd_index++;
    }
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */
int main(void)
{
    system_time_init();
    adc_init();
    i2c_sensor_init();
    gpio_status_init();
    fault_manager_init();
    telemetry_init();
    serial_protocol_init();

    char pkt_buf[TELEMETRY_BUFFER_SIZE];

    /* Run 30 iterations for the simulation demo.
     * On real hardware: replace with while(1) {}.  */
    for (int iteration = 0; iteration < 30; iteration++)
    {
        uint32_t loop_start = system_time_ms();

        /* 1. Check for serial commands */
        poll_serial();

        /* 2. Read ADC */
        uint16_t adc_raw = adc_read_raw();
        uint16_t adc_mv  = adc_raw_to_mv(adc_raw);

        /* 3. Read button */
        bool btn = button_is_pressed();

        /* 4. Read I2C sensor */
        float sx = 0.0f, sy = 0.0f, sz = 0.0f;
        bool  sensor_ok = i2c_sensor_read(&sx, &sy, &sz);

        /* 5. Update fault manager */
        fault_manager_update_adc(adc_mv);
        fault_manager_update_sensor(sensor_ok);
        fault_manager_update_button(btn);
        fault_manager_update_latency(system_time_ms() - loop_start);

        /* 6. Build telemetry packet */
        uint32_t    seq        = telemetry_next_seq();
        uint32_t    now_ms     = system_time_ms();
        bool        has_faults = fault_manager_has_faults();
        const char *fault_str  = fault_manager_get_fault_string();

        int pkt_len = telemetry_build_packet(
            pkt_buf, TELEMETRY_BUFFER_SIZE,
            seq, now_ms, adc_mv,
            sx, sy, sz,
            btn, has_faults, fault_str);

        /* 7. Send packet if streaming is enabled */
        if (serial_protocol_streaming_enabled())
        {
            if (pkt_len > 0)
            {
                puts(pkt_buf);
                fflush(stdout);
            }
        }

        /* 8. Update LED status */
        led_set_healthy(!has_faults);
        led_set_fault(has_faults);

        /* 9. Wait for next telemetry period */
        uint32_t elapsed = system_time_ms() - loop_start;
        uint32_t period  = serial_protocol_get_period_ms();
        if (elapsed < period)
        {
            system_time_delay_ms(period - elapsed);
        }
    }

    return 0;
}
