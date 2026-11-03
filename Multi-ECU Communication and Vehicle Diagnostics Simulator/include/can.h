#ifndef CAN_H
#define CAN_H

#include <linux/can.h>

/**
 * @brief Opens and binds a raw CAN socket to the given SocketCAN interface.
 * @param interface_name Name of the interface (e.g. "vcan0" or "can0")
 * @return Socket file descriptor on success, -1 on error
 */
int can_init(const char *interface_name);

/**
 * @brief Sends a CAN frame with the given ID and payload.
 * @param socket_fd Socket returned by can_init()
 * @param id CAN identifier
 * @param data Pointer to the payload bytes (max 8 bytes)
 * @param len Number of payload bytes (0-8)
 * @return 0 on success, -1 on error
 */
int can_send(int socket_fd, unsigned int id, unsigned char *data, unsigned char len);

/**
 * @brief Blocks until a CAN frame is received.
 * @param socket_fd Socket returned by can_init()
 * @param frame Pointer to a struct can_frame to be filled in
 * @return 0 on success, -1 on error
 */
int can_receive(int socket_fd, struct can_frame *frame);

/**
 * @brief Switches a socket to non-blocking mode.
 * @param socket_fd Socket to modify
 * @return 0 on success, -1 on error
 */
int can_set_nonblocking(int socket_fd);

/**
 * @brief Checks for a waiting frame without blocking the calling thread.
 * @param socket_fd Socket previously set non-blocking via can_set_nonblocking()
 * @param frame Pointer to a struct can_frame to be filled in if a frame arrived
 * @return 1 if a frame was received, 0 if none was available, -1 on real error
 */
int can_receive_nonblocking(int socket_fd, struct can_frame *frame);

/** @brief Closes a CAN socket. */
void can_close(int socket_fd);

/**
 * @brief Encodes an integer (up to 16 bits) into 2 big-endian bytes.
 * @param value Value to encode
 * @param data Output buffer, must have space for 2 bytes
 */
void can_encode_uint16(int value, unsigned char *data);

/**
 * @brief Decodes 2 big-endian bytes back into an integer.
 * @param data Input buffer containing 2 bytes
 * @return Decoded integer value
 */
int can_decode_uint16(const unsigned char *data);

/**
 * @brief Encodes an integer (0-255) into 1 byte.
 * @param value Value to encode
 * @param data Output buffer, must have space for 1 byte
 */
void can_encode_uint8(int value, unsigned char *data);

/**
 * @brief Decodes 1 byte back into an integer.
 * @param data Input buffer containing 1 byte
 * @return Decoded integer value (0-255)
 */
int can_decode_uint8(const unsigned char *data);

/**
 * @brief Validates that a data length code is within the CAN maximum.
 * @param len Length to validate
 * @return 1 if valid (0-8), 0 if invalid
 */
int can_validate_dlc(unsigned char len);

#endif