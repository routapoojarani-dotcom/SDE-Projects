#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "can.h"
#include "can_ids.h"
#include "logger.h"
#include "fault_injection.h"

long now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000L) + (ts.tv_nsec / 1000000L);
}

int main(void) {
    int s = can_init("vcan0");
    if (s < 0) {
        return 1;
    }

    can_set_nonblocking(s);
    srand(time(NULL));

    logger_init("engine_ecu");
    fault_init();

    // --- Physics state ---
    float speed = 0.0f;       // km/h, tracked as a float for smooth integration
    int rpm = 700;
    int temp = 20;
    int fuel = 100;

    int brake_pressed = 0;
    int accelerator_pressed = 0;

    const float MAX_SPEED = 200.0f;
    const float ACCEL_RATE = 12.0f;   // km/h per second while accelerating
    const float BRAKE_RATE = -20.0f;  // km/h per second while braking (negative = slowing down)
    const float COAST_RATE = -3.0f;   // km/h per second when coasting, neither pedal pressed

    // --- Intervals (ms) ---
    const long RPM_INTERVAL   = 20;
    const long SPEED_INTERVAL = 50;
    const long TEMP_INTERVAL  = 500;
    const long FUEL_INTERVAL  = 1000;
    const long PRINT_INTERVAL = 500;

    long last_rpm_sent   = 0;
    long last_speed_sent = 0;
    long last_temp_sent  = 0;
    long last_fuel_sent  = 0;
    long last_print      = 0;

    printf("Engine ECU started. Broadcasting on vcan0...\n");

    while (1) {
        long t = now_ms();

        // --- Check for brake/accelerator frames, non-blocking ---
        struct can_frame incoming;
        int result = can_receive_nonblocking(s, &incoming);
        if (result == 1) {
            if (incoming.can_id == ID_BRAKE_STATUS) {
                brake_pressed = incoming.data[0];
            } else if (incoming.can_id == ID_ACCELERATOR) {
                accelerator_pressed = incoming.data[0];
            }
        }

        // --- Speed: integrated from an acceleration rate, brake takes priority ---
        if (t - last_speed_sent >= SPEED_INTERVAL) {
            float dt = (t - last_speed_sent) / 1000.0f;  // elapsed time in seconds
            if (last_speed_sent == 0) dt = SPEED_INTERVAL / 1000.0f;  // first tick, avoid a huge dt

            float rate;
            if (brake_pressed) {
                rate = BRAKE_RATE;
            } else if (accelerator_pressed) {
                rate = ACCEL_RATE;
            } else {
                rate = COAST_RATE;
            }

            speed += rate * dt;
            if (speed < 0.0f) speed = 0.0f;
            if (speed > MAX_SPEED) speed = MAX_SPEED;

            int speed_int = (int)(speed + 0.5f);  // round to nearest whole km/h for transmission
            unsigned char speed_data[1];
            can_encode_uint8(speed_int, speed_data);

            if (!fault_should_drop()) {
                if (fault_should_corrupt()) fault_corrupt_data(speed_data, 1);
                int delay = fault_get_delay_ms();
                if (delay > 0) usleep(delay * 1000);

                can_send(s, ID_VEHICLE_SPEED, speed_data, 1);
                if (fault_should_duplicate()) can_send(s, ID_VEHICLE_SPEED, speed_data, 1);
            }
            last_speed_sent = t;
        }

        // --- RPM: derived from current speed, not its own independent random walk ---
        if (t - last_rpm_sent >= RPM_INTERVAL) {
            int speed_int = (int)(speed + 0.5f);

            // Base RPM scales with speed: idle (700) at 0 km/h, up to ~6000 at top speed
            int target_rpm = 700 + (int)(speed_int * ((6000.0f - 700.0f) / MAX_SPEED));

            // Small jitter so it doesn't look robotically perfect, more noticeable near idle
            int jitter = (speed_int < 5) ? ((rand() % 60) - 30) : ((rand() % 20) - 10);
            rpm = target_rpm + jitter;

            if (rpm < 700) rpm = 700;
            if (rpm > 6000) rpm = 6000;

            unsigned char rpm_data[2];
            can_encode_uint16(rpm, rpm_data);

            if (!fault_should_drop()) {
                if (fault_should_corrupt()) fault_corrupt_data(rpm_data, 2);
                int delay = fault_get_delay_ms();
                if (delay > 0) usleep(delay * 1000);

                can_send(s, ID_ENGINE_RPM, rpm_data, 2);
                if (fault_should_duplicate()) can_send(s, ID_ENGINE_RPM, rpm_data, 2);
            }
            last_rpm_sent = t;
        }

        // --- Temperature: unchanged, unaffected by pedals ---
        if (t - last_temp_sent >= TEMP_INTERVAL) {
            if (temp < 90) {
                temp += 1;
            } else {
                temp += (rand() % 3) - 1;
            }

            unsigned char temp_data[1];
            can_encode_uint8(temp, temp_data);

            if (!fault_should_drop()) {
                if (fault_should_corrupt()) fault_corrupt_data(temp_data, 1);
                int delay = fault_get_delay_ms();
                if (delay > 0) usleep(delay * 1000);

                can_send(s, ID_ENGINE_TEMP, temp_data, 1);
                if (fault_should_duplicate()) can_send(s, ID_ENGINE_TEMP, temp_data, 1);
            }
            last_temp_sent = t;
        }

        // --- Fuel: unchanged, consumed slowly over time ---
        if (t - last_fuel_sent >= FUEL_INTERVAL) {
            if (rand() % 5 == 0 && fuel > 0) {
                fuel -= 1;
            }

            unsigned char fuel_data[1];
            can_encode_uint8(fuel, fuel_data);

            if (!fault_should_drop()) {
                if (fault_should_corrupt()) fault_corrupt_data(fuel_data, 1);
                int delay = fault_get_delay_ms();
                if (delay > 0) usleep(delay * 1000);

                can_send(s, ID_FUEL_LEVEL, fuel_data, 1);
                if (fault_should_duplicate()) can_send(s, ID_FUEL_LEVEL, fuel_data, 1);
            }
            last_fuel_sent = t;
        }

        if (t - last_print >= PRINT_INTERVAL) {
            printf("RPM=%d Speed=%.1f Temp=%d Fuel=%d Brake=%s Accel=%s\n",
                   rpm, speed, temp, fuel,
                   brake_pressed ? "ON" : "off",
                   accelerator_pressed ? "ON" : "off");
            last_print = t;
        }

        usleep(5000);
    }

    logger_close();
    can_close(s);
    return 0;
}