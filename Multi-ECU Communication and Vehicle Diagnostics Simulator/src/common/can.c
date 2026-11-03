#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <fcntl.h>
#include <errno.h> 

#include "can.h"
#include "logger.h"

int can_init(const char *interface_name) {
    int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("can_init: socket");
        return -1;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);

    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("can_init: ioctl");
        close(s);
        return -1;
    }

    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("can_init: bind");
        close(s);
        return -1;
    }

    return s;
}

int can_send(int socket_fd, unsigned int id, unsigned char *data, unsigned char len) {
    if (!can_validate_dlc(len)) {
        fprintf(stderr, "can_send: data length %d exceeds CAN max of 8 bytes\n", len);
        return -1;
    }

    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));
    frame.can_id = id;
    frame.can_dlc = len;
    memcpy(frame.data, data, len);

    if (write(socket_fd, &frame, sizeof(frame)) != sizeof(frame)) {
        perror("can_send: write");
        return -1;
    }

    logger_log_tx(id, data, len);  // NEW

    return 0;
}

int can_receive(int socket_fd, struct can_frame *frame) {
    int nbytes = read(socket_fd, frame, sizeof(struct can_frame));
    if (nbytes < 0) {
        perror("can_receive: read");
        return -1;
    }

    logger_log_rx(frame->can_id, frame->data, frame->can_dlc);  // NEW

    return 0;
}

void can_close(int socket_fd) {
    close(socket_fd);
}

int can_set_nonblocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) {
        perror("can_set_nonblocking: fcntl get");
        return -1;
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("can_set_nonblocking: fcntl set");
        return -1;
    }
    return 0;
}

int can_receive_nonblocking(int socket_fd, struct can_frame *frame) {
    int nbytes = read(socket_fd, frame, sizeof(struct can_frame));
    if (nbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;  // nothing waiting right now — totally normal
        }
        perror("can_receive_nonblocking: read");
        return -1;
    }
    return 1;
}

void can_encode_uint16(int value, unsigned char *data) {
    data[0] = (value >> 8) & 0xFF;
    data[1] = value & 0xFF;
}

int can_decode_uint16(const unsigned char *data) {
    return (data[0] << 8) | data[1];
}

void can_encode_uint8(int value, unsigned char *data) {
    data[0] = value & 0xFF;
}

int can_decode_uint8(const unsigned char *data) {
    return data[0];
}

int can_validate_dlc(unsigned char len) {
    return (len <= 8) ? 1 : 0;
}