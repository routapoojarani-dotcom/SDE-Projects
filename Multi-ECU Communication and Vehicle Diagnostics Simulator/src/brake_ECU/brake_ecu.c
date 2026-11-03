#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "can.h"
#include "can_ids.h"
#include "logger.h"
#include "fault_injection.h"

// Reads and discards characters up to and including the next newline.
// Used after a letter command, to swallow the Enter key that follows it.
static void flush_line(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        // discard
    }
}

int main(void) {
    int s = can_init("vcan0");
    if (s < 0) {
        return 1;
    }

    srand(time(NULL));
    logger_init("brake_ecu");
    fault_init();

    int brake_on = 0;
    int accelerator_on = 0;

    printf("Brake / Accelerator ECU started.\n");
    printf("ENTER = toggle brake, 'a' + ENTER = toggle accelerator, 'q' + ENTER = quit.\n");

    while (1) {
        int c = getchar();

        if (c == 'q') {
            break;
        } else if (c == '\n') {
            brake_on = !brake_on;
            unsigned char data[1] = { (unsigned char)brake_on };

            if (!fault_should_drop()) {
                if (fault_should_corrupt()) fault_corrupt_data(data, 1);
                int delay = fault_get_delay_ms();
                if (delay > 0) usleep(delay * 1000);

                can_send(s, ID_BRAKE_STATUS, data, 1);
                if (fault_should_duplicate()) can_send(s, ID_BRAKE_STATUS, data, 1);
            }
            printf("Brake %s -> frame sent\n", brake_on ? "PRESSED" : "RELEASED");

        } else if (c == 'a') {
            flush_line();  // NEW: swallow the Enter key that follows 'a', so it isn't read as a bare '\n' next

            accelerator_on = !accelerator_on;
            unsigned char data[1] = { (unsigned char)accelerator_on };

            if (!fault_should_drop()) {
                if (fault_should_corrupt()) fault_corrupt_data(data, 1);
                int delay = fault_get_delay_ms();
                if (delay > 0) usleep(delay * 1000);

                can_send(s, ID_ACCELERATOR, data, 1);
                if (fault_should_duplicate()) can_send(s, ID_ACCELERATOR, data, 1);
            }
            printf("Accelerator %s -> frame sent\n", accelerator_on ? "PRESSED" : "RELEASED");
        }
    }

    printf("Brake / Accelerator ECU shutting down.\n");
    logger_close();
    can_close(s);
    return 0;
}