#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

int main(void) {
    // 1. Create a raw CAN socket
    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("socket");
        return 1;
    }

    // 2. Find the interface index for "vcan0"
    struct ifreq ifr;
    strcpy(ifr.ifr_name, "vcan0");
    ioctl(s, SIOCGIFINDEX, &ifr);

    // 3. Bind the socket to vcan0
    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    printf("Listening on vcan0... (Ctrl+C to stop)\n");

    // 4. Loop forever, reading frames as they arrive
    struct can_frame frame;
    while (1) {
        int nbytes = read(s, &frame, sizeof(frame));
        if (nbytes < 0) {
            perror("read");
            break;
        }

        printf("Received frame: ID=0x%X DLC=%d Data=", frame.can_id, frame.can_dlc);
        for (int i = 0; i < frame.can_dlc; i++) {
            printf("%02X ", frame.data[i]);
        }
        printf("\n");
    }

    close(s);
    return 0;
}