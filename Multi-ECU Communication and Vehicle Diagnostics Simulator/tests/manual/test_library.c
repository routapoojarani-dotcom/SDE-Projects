#include <stdio.h>
#include "can.h"

int main(void) {
    int s = can_init("vcan0");
    if (s < 0) {
        return 1;
    }

    unsigned char data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    if (can_send(s, 0x321, data, 4) == 0) {
        printf("Sent frame via library, ID 0x321\n");
    }

    can_close(s);
    return 0;
}