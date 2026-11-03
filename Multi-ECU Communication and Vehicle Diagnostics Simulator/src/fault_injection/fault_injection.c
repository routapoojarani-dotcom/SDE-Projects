#include <stdlib.h>

#include "fault_injection.h"

#define DROP_PERCENT      5
#define DUPLICATE_PERCENT 3
#define CORRUPT_PERCENT   4
#define MAX_DELAY_MS      200

static int faults_enabled = 0;  // NEW: off by default

void fault_init(void) {
    // NEW: only enable faults if the environment variable is explicitly set
    const char *env = getenv("FAULT_INJECTION_ENABLED");
    faults_enabled = (env != NULL && env[0] == '1');
}

int fault_should_drop(void) {
    if (!faults_enabled) return 0;
    return (rand() % 100) < DROP_PERCENT;
}

int fault_should_duplicate(void) {
    if (!faults_enabled) return 0;
    return (rand() % 100) < DUPLICATE_PERCENT;
}

int fault_should_corrupt(void) {
    if (!faults_enabled) return 0;
    return (rand() % 100) < CORRUPT_PERCENT;
}

void fault_corrupt_data(unsigned char *data, unsigned char len) {
    if (len == 0) return;
    int byte_index = rand() % len;
    data[byte_index] = (unsigned char)(rand() % 256);
}

int fault_get_delay_ms(void) {
    if (!faults_enabled) return 0;
    if ((rand() % 100) < 10) {
        return rand() % MAX_DELAY_MS;
    }
    return 0;
}