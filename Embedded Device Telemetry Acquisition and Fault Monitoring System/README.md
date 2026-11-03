# Embedded Device Telemetry Acquisition and Fault Monitoring System

A modular embedded telemetry system built in C (STM32 firmware) and Python (host application).
The firmware streams structured UART telemetry packets; the host app parses, logs, analyzes, and reports on them.

---

## Project Overview

| Layer | Technology | Purpose |
|-------|-----------|---------|
| Firmware | C99, STM32 HAL | Sample sensors, detect faults, stream JSON telemetry |
| Transport | USB serial / UART 115200 bps | Newline-delimited JSON packets |
| Host app | Python 3.10+ | Parse, log to CSV/JSONL, live dashboard, Markdown report |

**No hardware required to run the Python pipeline** — use `simulate` mode to generate fake telemetry.

---

## Hardware

- STM32 Nucleo-F401RE (or compatible Nucleo board)
- 10 kΩ potentiometer on PA0 (ADC input)
- B1 USER push button on PC13 (onboard)
- Green LED LD2 on PA5 (onboard)
- Red LED on PB3 with 220 Ω series resistor
- MPU-6050 or BME280 on I2C1 (PB8/PB9)

See [docs/wiring.md](docs/wiring.md) for full wiring diagrams.

---

## Software Stack

- **Firmware**: C99, `stdio.h`/`string.h` only (portable; swaps to STM32 HAL for hardware)
- **Host**: Python 3.10+, pyserial, pandas, matplotlib, pytest
- **Protocol**: Newline-delimited JSON — human-readable, easily parsed

---

## Documentation

| File | Contents |
|------|---------|
| [docs/architecture.md](docs/architecture.md) | Firmware + host architecture, data flow |
| [docs/serial_protocol.md](docs/serial_protocol.md) | Packet schema, commands, fault types |
| [docs/test_plan.md](docs/test_plan.md) | Unit tests, hardware tests, acceptance criteria |
| [docs/wiring.md](docs/wiring.md) | STM32 Nucleo wiring for all peripherals |
