#include "serial_protocol.h"
#include "fault_manager.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MIN_PERIOD_MS   10u
#define MAX_PERIOD_MS 5000u

static bool     s_streaming = false;
static uint32_t s_period_ms = DEFAULT_PERIOD_MS;

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static void send(const char *json)
{
    puts(json);
    fflush(stdout);
}

/* Strip trailing whitespace in-place and return pointer past any leading
 * command name. "SET_RATE 100" -> name="SET_RATE", returns "100". */
static const char *split_args(const char *line, char *name_buf, size_t name_buf_sz)
{
    const char *space = strchr(line, ' ');
    if (space)
    {
        size_t n = (size_t)(space - line);
        if (n >= name_buf_sz) n = name_buf_sz - 1u;
        strncpy(name_buf, line, n);
        name_buf[n] = '\0';
        return space + 1;
    }
    strncpy(name_buf, line, name_buf_sz - 1u);
    name_buf[name_buf_sz - 1u] = '\0';
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Command handlers                                                    */
/* ------------------------------------------------------------------ */

static void handle_ping(const char *args)
{
    (void)args;
    send("{\"type\":\"response\",\"cmd\":\"PING\",\"ok\":true}");
}

static void handle_start(const char *args)
{
    (void)args;
    s_streaming = true;
    send("{\"type\":\"response\",\"cmd\":\"START\",\"ok\":true,\"streaming\":true}");
}

static void handle_stop(const char *args)
{
    (void)args;
    s_streaming = false;
    send("{\"type\":\"response\",\"cmd\":\"STOP\",\"ok\":true,\"streaming\":false}");
}

static void handle_get_status(const char *args)
{
    (void)args;
    bool        has_faults = fault_manager_has_faults();
    const char *fault_str  = fault_manager_get_fault_string();
    char resp[512];
    snprintf(resp, sizeof(resp),
        "{\"type\":\"response\",\"cmd\":\"GET_STATUS\","
        "\"ok\":true,\"status\":\"%s\",\"faults\":[%s]}",
        has_faults ? "FAULT" : "OK",
        fault_str);
    send(resp);
}

static void handle_get_sensor(const char *args)
{
    (void)args;
    send("{\"type\":\"response\",\"cmd\":\"GET_SENSOR\",\"ok\":true}");
}

static void handle_set_rate(const char *args)
{
    if (!args || args[0] == '\0')
    {
        send("{\"type\":\"response\",\"cmd\":\"SET_RATE\","
             "\"ok\":false,\"error\":\"MISSING_PERIOD\"}");
        return;
    }
    long n = strtol(args, NULL, 10);
    if (n < (long)MIN_PERIOD_MS || n > (long)MAX_PERIOD_MS)
    {
        send("{\"type\":\"response\",\"cmd\":\"SET_RATE\","
             "\"ok\":false,\"error\":\"INVALID_PERIOD\"}");
        return;
    }
    s_period_ms = (uint32_t)n;
    char resp[128];
    snprintf(resp, sizeof(resp),
        "{\"type\":\"response\",\"cmd\":\"SET_RATE\","
        "\"ok\":true,\"period_ms\":%lu}",
        (unsigned long)s_period_ms);
    send(resp);
}

static void handle_clear_faults(const char *args)
{
    (void)args;
    fault_manager_clear();
    send("{\"type\":\"response\",\"cmd\":\"CLEAR_FAULTS\",\"ok\":true}");
}

static void handle_reset_stats(const char *args)
{
    (void)args;
    send("{\"type\":\"response\",\"cmd\":\"RESET_STATS\",\"ok\":true}");
}

/* ------------------------------------------------------------------ */
/* Command dispatch table                                              */
/* ------------------------------------------------------------------ */

typedef void (*CmdHandler)(const char *args);

typedef struct { const char *name; CmdHandler handler; } CmdEntry;

static const CmdEntry k_commands[] = {
    { "PING",         handle_ping         },
    { "START",        handle_start        },
    { "STOP",         handle_stop         },
    { "GET_STATUS",   handle_get_status   },
    { "GET_SENSOR",   handle_get_sensor   },
    { "SET_RATE",     handle_set_rate     },
    { "CLEAR_FAULTS", handle_clear_faults },
    { "RESET_STATS",  handle_reset_stats  },
};
#define CMD_COUNT (sizeof(k_commands) / sizeof(k_commands[0]))

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

void serial_protocol_init(void)
{
    s_streaming = false;
    s_period_ms = DEFAULT_PERIOD_MS;
}

void serial_protocol_handle_line(const char *line)
{
    if (!line || line[0] == '\0') return;

    char       name[32];
    const char *args = split_args(line, name, sizeof(name));

    for (size_t i = 0; i < CMD_COUNT; i++)
    {
        if (strcmp(name, k_commands[i].name) == 0)
        {
            k_commands[i].handler(args);
            return;
        }
    }

    send("{\"type\":\"response\",\"cmd\":\"UNKNOWN\","
         "\"ok\":false,\"error\":\"INVALID_COMMAND\"}");
}

bool serial_protocol_streaming_enabled(void)
{
    return s_streaming;
}

uint32_t serial_protocol_get_period_ms(void)
{
    return s_period_ms;
}
