#ifndef LOGGER_H
#define LOGGER_H

/**
 * @brief Initializes logging for this ECU. Opens/creates logs/<ecu_name>.log in append mode.
 * @param ecu_name Identifier used in the log filename (e.g. "engine_ecu")
 */
void logger_init(const char *ecu_name);

/**
 * @brief Logs a transmitted CAN frame with a timestamp.
 * @param id CAN identifier
 * @param data Payload bytes
 * @param len Number of payload bytes
 */
void logger_log_tx(unsigned int id, unsigned char *data, unsigned char len);

/**
 * @brief Logs a received CAN frame with a timestamp.
 * @param id CAN identifier
 * @param data Payload bytes
 * @param len Number of payload bytes
 */
void logger_log_rx(unsigned int id, unsigned char *data, unsigned char len);

/** @brief Flushes and closes the log file. Should be called before the ECU exits. */
void logger_close(void);

#endif