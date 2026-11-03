#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "can.h"
#include "can_ids.h"
#include "logger.h"

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
    logger_init("dashboard_ecu");

    printf("Dashboard ECU started. Listening on vcan0...\n");

    struct can_frame frame;

    const long PRINT_INTERVAL = 500;  // only print each signal at most twice a second
    long last_rpm_print = 0;
    long last_speed_print = 0;
    long last_temp_print = 0;
    long last_fuel_print = 0;
    // Brake and turn signal are event-driven already (rare), so they always print

    while (1) {
        if (can_receive(s, &frame) != 0) {
            continue;
        }

        long t = now_ms();

        switch (frame.can_id) {
            case ID_ENGINE_RPM: {
                int rpm = can_decode_uint16(frame.data);
                if (t - last_rpm_print >= PRINT_INTERVAL) {
                    printf("[Dashboard] RPM: %d\n", rpm);
                    last_rpm_print = t;
                }
                break;
            }
            case ID_VEHICLE_SPEED: {
                int speed = can_decode_uint8(frame.data);
                if (t - last_speed_print >= PRINT_INTERVAL) {
                    printf("[Dashboard] Speed: %d km/h\n", speed);
                    last_speed_print = t;
                }
                break;
            }
            case ID_ENGINE_TEMP: {
                int temp = can_decode_uint8(frame.data);
                if (t - last_temp_print >= PRINT_INTERVAL) {
                    printf("[Dashboard] Engine Temp: %d C\n", temp);
                    last_temp_print = t;
                }
                break;
            }
            case ID_FUEL_LEVEL: {
                int fuel = can_decode_uint8(frame.data);
                if (t - last_fuel_print >= PRINT_INTERVAL) {
                    printf("[Dashboard] Fuel: %d%%\n", fuel);
                    last_fuel_print = t;
                }
                break;
            }
            case ID_BRAKE_STATUS: {
                int brake = can_decode_uint8(frame.data);
                printf("[Dashboard] Brake: %s\n", brake ? "PRESSED (warning icon ON)" : "released");
                break;
            }
            case ID_TURN_SIGNAL: {
                int signal = can_decode_uint8(frame.data);
                const char *label;
                switch (signal) {
                    case 0: label = "off"; break;
                    case 1: label = "LEFT"; break;
                    case 2: label = "RIGHT"; break;
                    case 3: label = "HAZARDS"; break;
                    default: label = "unknown"; break;
                }
                printf("[Dashboard] Turn signal: %s\n", label);
                break;
            }
            case ID_ACCELERATOR: {
                int accel = frame.data[0];
                printf("[Dashboard] Accelerator: %s\n", accel ? "PRESSED" : "released");
                break;
            }
            default:
                break;
        }
    }

    can_close(s);
    return 0;
}