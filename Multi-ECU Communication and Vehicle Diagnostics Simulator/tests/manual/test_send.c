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

    // 4. Build a CAN frame
    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));
    frame.can_id = 0x123;
    frame.can_dlc = 4;
    frame.data[0] = 0xDE;
    frame.data[1] = 0xAD;
    frame.data[2] = 0xBE;
    frame.data[3] = 0xEF;

    // 5. Send it
    if (write(s, &frame, sizeof(frame)) != sizeof(frame)) {
        perror("write");
        return 1;
    }

    printf("Frame sent on vcan0, ID 0x%X\n", frame.can_id);

    close(s);
    return 0;
}