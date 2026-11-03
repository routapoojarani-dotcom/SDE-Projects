#include <stdio.h>
#include <unistd.h>

#include "can.h"
#include "can_ids.h"
#include "logger.h"

int main(void) {
    int s = can_init("vcan0");
    if (s < 0) {
        return 1;
    }
    logger_init("lighting_ecu");

    can_set_nonblocking(s);

    int brake_on = 0;
    int turn_signal = 0;      // 0=off, 1=left, 2=right, 3=hazards
    int blink_state = 0;      // toggles each loop to create blinking
    int blink_counter = 0;

    printf("Lighting ECU started. Watching brake + turn signal frames...\n");

    while (1) {
        struct can_frame frame;
        int result = can_receive_nonblocking(s, &frame);

        if (result == 1) {
            if (frame.can_id == ID_BRAKE_STATUS) {
                brake_on = frame.data[0];
                printf("[Lighting] Brake lights: %s\n", brake_on ? "on" : "off");
            } else if (frame.can_id == ID_TURN_SIGNAL) {
                turn_signal = frame.data[0];
                blink_counter = 0;  // restart blink timing whenever signal changes
            }
        }

        // --- Handle blinking, independent of whether a new frame arrived ---
        blink_counter++;
        if (blink_counter >= 5) {  // roughly every 5 loop ticks, flip blink state
            blink_state = !blink_state;
            blink_counter = 0;

            if (turn_signal == 1) {
                printf("[Lighting] Left indicator: %s\n", blink_state ? "on" : "off");
            } else if (turn_signal == 2) {
                printf("[Lighting] Right indicator: %s\n", blink_state ? "on" : "off");
            } else if (turn_signal == 3) {
                printf("[Lighting] Hazards: %s\n", blink_state ? "on" : "off");
            }
        }

        usleep(100000);  // 100ms tick — determines blink speed and CPU usage
    }

    can_close(s);
    return 0;
}