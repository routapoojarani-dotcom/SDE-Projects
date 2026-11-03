#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "logger.h"

static FILE *log_file = NULL;

void logger_init(const char *ecu_name) {
    mkdir("logs", 0755);  // create logs/ folder if it doesn't exist yet; ignore error if it already does

    char path[256];
    snprintf(path, sizeof(path), "logs/%s.log", ecu_name);

    log_file = fopen(path, "a");  // "a" = append, so restarting an ECU doesn't erase past logs
    if (log_file == NULL) {
        perror("logger_init: fopen");
    }
}

// Internal helper: writes one log line with timestamp, direction, ID, and data bytes
static void log_frame(const char *direction, unsigned int id, unsigned char *data, unsigned char len) {
    if (log_file == NULL) {
        return;  // logging not initialized — fail silently rather than crash
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    fprintf(log_file, "[%ld.%03ld] %s ID=0x%03X DLC=%d DATA=",
            ts.tv_sec, ts.tv_nsec / 1000000, direction, id, len);

    for (int i = 0; i < len; i++) {
        fprintf(log_file, "%02X ", data[i]);
    }
    fprintf(log_file, "\n");

    fflush(log_file);  // write immediately, don't wait for buffer to fill — important since ECUs run forever
}

void logger_log_tx(unsigned int id, unsigned char *data, unsigned char len) {
    log_frame("TX", id, data, len);
}

void logger_log_rx(unsigned int id, unsigned char *data, unsigned char len) {
    log_frame("RX", id, data, len);
}

void logger_close(void) {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
}