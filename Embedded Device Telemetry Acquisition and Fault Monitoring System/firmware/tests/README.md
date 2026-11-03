# Firmware Tests

Firmware unit tests are currently planned for the following modules:

- `fault_manager` — threshold checks, bitmask logic, string builder
- `telemetry` — packet formatting, buffer overflow protection
- `serial_protocol` — command parsing, streaming state machine

## Running the Simulation

Build and run the firmware simulation on a host PC:

```bash
cd firmware
make
./firmware_sim
```

This runs 30 simulated telemetry loop iterations and prints JSON packets to stdout — directly parsable by the Python host.

## Hardware Testing

See `docs/test_plan.md` for hardware validation steps on the STM32 Nucleo board.
