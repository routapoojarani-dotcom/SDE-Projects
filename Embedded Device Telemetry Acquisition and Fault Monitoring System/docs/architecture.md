# System Architecture

## Overview

The Embedded Telemetry Logger is split into two software layers:

1. **Firmware** — runs on an STM32 Nucleo microcontroller (or simulated on host PC)
2. **Python Host** — runs on a laptop, receives and processes telemetry via USB serial

```
┌─────────────────────────────────┐         USB / UART
│         STM32 Nucleo            │ ─────────────────────> ┌──────────────────────┐
│                                 │ JSON newline packets    │   Python Host App    │
│  ADC → sensor_adc               │                         │                      │
│  I2C → sensor_i2c               │ <─────────────────────  │  serial_client.py    │
│  BTN → gpio_status              │ Commands (PING/START…)  │  packet_parser.py    │
│        ↓                        │                         │  csv_logger.py       │
│  fault_manager                  │                         │  json_logger.py      │
│        ↓                        │                         │  stats.py            │
│  telemetry → serial_protocol    │                         │  report_generator.py │
│        ↓                        │                         │  dashboard.py        │
│  UART TX (JSON lines)           │                         └──────────────────────┘
└─────────────────────────────────┘
```

---

## Firmware Architecture

### Module Responsibilities

| Module | Responsibility |
|--------|---------------|
| `sensor_adc` | Initialize ADC, read raw 12-bit value, convert to millivolts |
| `sensor_i2c` | Initialize I2C bus, read 3-axis accelerometer (MPU-6050 compatible) |
| `gpio_status` | Read button state, drive green (healthy) and red (fault) LEDs |
| `fault_manager` | Maintain bitmask of active faults, build JSON-formatted fault string |
| `telemetry` | Format sensor data + faults into a newline-delimited JSON packet |
| `serial_protocol` | Parse incoming host commands, dispatch to handlers, send JSON responses |
| `system_time` | Provide millisecond timestamp; simulate HAL_GetTick() / HAL_Delay() |
| `main` | Initialize all modules, run the 100 ms telemetry loop |

### Main Loop (100 ms period)

```
loop_start = system_time_ms()

1. poll_serial()              — check UART RX for host commands
2. adc_read_raw() / adc_raw_to_mv()  — read potentiometer
3. button_is_pressed()        — read user button
4. i2c_sensor_read(&x, &y, &z) — read IMU or BME280
5. fault_manager_update_*()   — update all fault conditions
6. telemetry_build_packet()   — format JSON telemetry string
7. puts(pkt_buf) if streaming — transmit over UART/USB
8. led_set_healthy / led_set_fault — update status LEDs
9. delay(period - elapsed)    — maintain precise sample rate
```

### Fault Bitmask Design

The fault manager uses a `uint8_t` bitmask. Each `update_*()` function sets or clears one bit. A descriptor table maps each bit to its JSON-quoted label string. This approach adds a new fault type with one enum constant + one table entry — no switch statements to update.

---

## Python Host Architecture

### Module Responsibilities

| Module | Responsibility |
|--------|---------------|
| `packet_parser` | Parse and validate JSON lines; raise `ValueError` on bad input |
| `serial_client` | Own serial port lifecycle; call `parse_packet`; swallow bad packets |
| `simulator` | Generate realistic fake telemetry packets for testing without hardware |
| `csv_logger` | Append telemetry to CSV; faults stored as semicolon-separated string |
| `json_logger` | Append telemetry to JSONL; faults preserved as JSON array |
| `stats` | Pure functions: packet loss, average rate, fault counts, ADC range |
| `report_generator` | Consume stats output; write Markdown report |
| `dashboard` | Live console display; carriage-return line overwrite |
| `cli` | argparse CLI; wires modules together for `run`, `simulate`, `report` |

### Data Flow: Sensor to CSV/Report

```
STM32 ADC
    │
    ├─ sensor_adc.c → raw → millivolts
    │
    ├─ fault_manager.c → fault bitmask → fault string
    │
    ├─ telemetry.c → JSON packet string → UART TX
    │
    │         (USB serial cable)
    │
    ├─ serial_client.py → read_line() → parse_packet()
    │
    ├─ csv_logger.py  → sample_run.csv
    ├─ json_logger.py → sample_run.jsonl
    │
    ├─ stats.py       → loss, rate, faults, ADC range
    │
    └─ report_generator.py → run_report.md
```

### Dependency Graph (no circular deps)

```
simulator    ← (no deps inside package)
packet_parser ← (json only)
stats         ← (no deps inside package)
csv_logger    ← (csv, datetime, pathlib)
json_logger   ← (json, datetime, pathlib)
report_generator ← stats
dashboard     ← stats
serial_client ← packet_parser, pyserial
cli           ← all of the above
```
