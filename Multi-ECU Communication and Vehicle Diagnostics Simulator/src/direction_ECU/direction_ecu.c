#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "can.h"
#include "can_ids.h"
#include "logger.h"
#include "fault_injection.h"

// Small helper so we don't repeat the fault-wrapping block four times
static void send_signal(int s, unsigned char value) {
    unsigned char data[1] = { value };

    if (!fault_should_drop()) {
        if (fault_should_corrupt()) {
            fault_corrupt_data(data, 1);
        }
        int delay = fault_get_delay_ms();
        if (delay > 0) usleep(delay * 1000);

        can_send(s, ID_TURN_SIGNAL, data, 1);
        if (fault_should_duplicate()) {
            can_send(s, ID_TURN_SIGNAL, data, 1);
        }
    }
}

int main(void) {
    int s = can_init("vcan0");
    if (s < 0) {
        return 1;
    }

    srand(time(NULL));
    logger_init("direction_ecu");
    fault_init();

    printf("Direction ECU started.\n");
    printf("'l' + ENTER = left signal, 'r' + ENTER = right signal,\n");
    printf("'h' + ENTER = hazards, 'o' + ENTER = signals off, 'q' + ENTER = quit.\n");

    while (1) {
        int c = getchar();

        if (c == 'q') {
            break;
        } else if (c == 'l') {
            send_signal(s, 1);
            printf("Left signal ON -> frame sent\n");
        } else if (c == 'r') {
            send_signal(s, 2);
            printf("Right signal ON -> frame sent\n");
        } else if (c == 'h') {
            send_signal(s, 3);
            printf("Hazards ON -> frame sent\n");
        } else if (c == 'o') {
            send_signal(s, 0);
            printf("Turn signals OFF -> frame sent\n");
        }
    }

    printf("Direction ECU shutting down.\n");
    logger_close();
    can_close(s);
    return 0;
}