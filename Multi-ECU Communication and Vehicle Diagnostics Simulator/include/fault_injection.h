#ifndef FAULT_INJECTION_H
#define FAULT_INJECTION_H

/**
 * @brief Initializes fault injection. Reads the FAULT_INJECTION_ENABLED
 * environment variable; faults are only active if it's set to "1".
 */
void fault_init(void);

/** @brief @return 1 if the current frame should be dropped (not sent), 0 otherwise. */
int fault_should_drop(void);

/** @brief @return 1 if the current frame should be sent twice, 0 otherwise. */
int fault_should_duplicate(void);

/** @brief @return 1 if the current frame's data should be corrupted, 0 otherwise. */
int fault_should_corrupt(void);

/**
 * @brief Corrupts one random byte of the given data buffer in place.
 * @param data Buffer to corrupt
 * @param len Length of the buffer
 */
void fault_corrupt_data(unsigned char *data, unsigned char len);

/** @brief @return A random delay in milliseconds to apply before sending (0 = no delay). */
int fault_get_delay_ms(void);

#endif