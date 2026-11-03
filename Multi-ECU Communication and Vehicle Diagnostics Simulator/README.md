# Multi-ECU Communication and Vehicle Diagnostics Simulator

A software-only recreation of an automotive in-vehicle network, built in embedded C using SocketCAN on Linux. Five independent processes simulate real automotive ECUs (Engine, Dashboard, Brake, Direction, Lighting), communicating exclusively over a virtual CAN bus — no physical hardware required.

## Overview

Modern vehicles run dozens of small embedded computers (ECUs) that talk to each other constantly over the CAN protocol. This project recreates that architecture at a smaller scale: each ECU is a standalone Linux process, unaware of the others' internals, communicating only through CAN frames on a shared virtual bus (`vcan0`).

The project covers the full lifecycle of that idea — from raw socket programming, through a shared library, realistic timing, structured logging, configurable fault injection, and a tested, documented codebase.

## Architecture

src/
common/ can.c, logger.c — shared library used by every ECU
engine_ecu/ generates RPM, speed, temperature, fuel
dashboard_ecu/ listens to all signals, prints decoded state
brake_ecu/ simulated brake pedal (manual input)
direction_ecu/ simulated turn signals / hazards (manual input)
lighting_ecu/ reacts to brake + turn signal frames
fault_injection/ configurable packet loss, delay, corruption, duplication

include/ public headers (can.h, logger.h, fault_injection.h, can_ids.h)
tests/
unit/ unit tests for the CAN encode/decode library
manual/ early exploratory send/receive test programs
scripts/ stress test runner, performance analysis
logs/ runtime-generated logs (git-ignored)
docs/ Doxygen-generated API documentation

Every ECU links against the same `can.c`/`logger.c` library, so socket handling, encoding, and logging are written once and shared — no ECU touches a raw socket directly.

## Signals

| Signal | CAN ID | Type | Interval |
|---|---|---|---|
| Engine RPM | 0x100 | Periodic | 20 ms |
| Vehicle Speed | 0x101 | Periodic | 50 ms |
| Engine Temperature | 0x102 | Periodic | 500 ms |
| Fuel Level | 0x103 | Periodic | 1000 ms |
| Brake Status | 0x104 | Event-driven | On change |
| Turn Signal | 0x105 | Event-driven | On change |
| Accelerator | 0x106 | Event-driven | On change |

Periodic signals use independent transmission intervals rather than a single shared loop delay, so higher-priority, fast-changing signals (RPM, speed) update far more often than slow ones (fuel) — matching how real automotive networks are scheduled.

## Building

Requires a Linux environment (native or VM) with `gcc` and `can-utils` installed.

```bash
sudo apt install can-utils build-essential
```

Set up the virtual CAN interface (required once per boot session, since it doesn't persist across restarts):

```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

Compile each ECU:

```bash
gcc -I include -o build/engine_ecu     src/engine_ecu/engine_ecu.c     src/common/can.c src/common/logger.c src/fault_injection/fault_injection.c
gcc -I include -o build/dashboard_ecu  src/dashboard_ecu/dashboard_ecu.c src/common/can.c src/common/logger.c
gcc -I include -o build/brake_ecu      src/brake_ecu/brake_ecu.c      src/common/can.c src/common/logger.c src/fault_injection/fault_injection.c
gcc -I include -o build/direction_ecu  src/direction_ecu/direction_ecu.c src/common/can.c src/common/logger.c src/fault_injection/fault_injection.c
gcc -I include -o build/lighting_ecu   src/lighting_ecu/lighting_ecu.c src/common/can.c src/common/logger.c
```

## Running

Each ECU runs as its own process, typically in its own terminal:

```bash
./build/engine_ecu
./build/dashboard_ecu
./build/brake_ecu       # ENTER = toggle brake, 'a' + ENTER = toggle accelerator, 'q' + ENTER to quit
./build/direction_ecu   # 'l' / 'r' / 'h' / 'o' + ENTER, 'q' + ENTER to quit
./build/lighting_ecu
```

Dashboard and Lighting are pure listeners and require no input. Brake and Direction simulate driver actions via the keyboard.

## Fault Injection

Fault injection is off by default and controlled per-run via an environment variable, so it never interferes with normal testing:

```bash
FAULT_INJECTION_ENABLED=1 ./build/engine_ecu
```

When enabled, each transmitted frame has an independent chance of being dropped, duplicated, delayed, or having one data byte corrupted — simulating bus noise, congestion, and sensor glitches. Rates are defined as constants in `src/fault_injection/fault_injection.c`.

## Logging

Every ECU logs each transmitted and received frame to `logs/<ecu_name>.log`, with a timestamp, direction (TX/RX), CAN ID, and payload bytes. Logs are append-only across runs, so they build a persistent record of everything the ECU has ever sent or seen.

## Testing

**Unit tests** cover the CAN encoding/decoding library in isolation (no live bus required):

```bash
gcc -I include -o build/test_can_encoding tests/unit/test_can_encoding.c src/common/can.c src/common/logger.c
./build/test_can_encoding
```

9 tests covering round-trip encoding, byte-order correctness, boundary values, and overflow handling — all passing.

**Stress testing** runs all five ECUs concurrently, with fault injection active, over an extended period, while sampling memory usage every 30 seconds:

```bash
./scripts/stress_test.sh 1800   # duration in seconds
```

**Results from a 30-minute run:**
- Memory usage remained flat throughout (Engine ~1.5 MB, Dashboard ~1.4 MB, Lighting ~1.5 MB RSS), with no upward drift — no evidence of memory leaks.
- No process crashes or unexpected terminations.
- Engine ECU sustained a TX throughput of ~10.9 frames/sec with fault injection active; RX on Dashboard tracked closely at ~21.2 frames/sec (each Engine signal is received by both Dashboard and Lighting).
- Per-signal frame counts (RPM: 8,836 · Speed: 5,663 · Temp: 3,404 · Fuel: 2,592) confirm the interval-based scheduling is working as designed — RPM, the fastest signal, transmits roughly 3.4x more often than fuel, the slowest.

Performance figures are computed directly from log timestamps via `scripts/analyze_performance.py`, not estimated.

## Tools

- **gcc** — compilation
- **GDB** — debugging
- **Valgrind** — memory analysis
- **cppcheck** — static analysis
- **clang-format** — code formatting
- **Doxygen** — API documentation (see `docs/html/index.html` after running `doxygen Doxyfile`)
- **Git** — version control

## Web Visualization

A live, interactive browser-based visualization sits on top of the C backend, built as a separate layer that reads and writes real CAN frames — no ECU code was modified to support it.

**Architecture:**
[Engine/Brake/Direction ECUs] → vcan0 → [Python bridge, python-can] → WebSocket → [Browser: canvas + JS]
↑
[Browser buttons] → same bridge → CAN frames back onto vcan0

The bridge (`bridge/server.py`) opens `vcan0` directly using `python-can`'s `socketcan` backend — the same kernel interface the C ECUs use — and runs a background thread that reads frames and pushes them to any connected browser over a WebSocket (Flask-SocketIO). On-screen buttons send frames back the other way, via small Flask routes, landing in the real Brake, Accelerator, and Direction ECUs exactly as keyboard input would.

**Running it:**

```bash
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt

./scripts/setup_vcan.sh
python3 bridge/server.py
```

Then open `http://localhost:5000` in a browser (inside the VM, or forwarded to the host if networking is configured).

**Features:**
- Live HUD showing RPM, speed, and fuel, throttled to update twice a second for readability
- A 3-lane road with a car that smoothly banks and slides between lanes on turn-signal input
- Brake lights and a fading red ground-glow effect when braking
- Blinking turn signal indicators at the front corners, and a hazard light button
- Randomly spawning speed bumps and pits; crossing one too fast triggers a temporary speed penalty and an on-screen message, while crossing slowly passes without incident
- On-screen Accelerate/Brake (press-and-hold) and Left/Right/Hazard (tap-to-toggle) controls, plus a Pause/Resume button for the visualization itself

## Known Limitations

- **Obstacle collisions (speed bumps, pits) only affect the browser-side visualization**, not the real Engine ECU. A fast collision temporarily slows the on-screen road-scroll rate and triggers a visual/toast effect, but the actual `speed` value broadcast on the CAN bus is untouched. Wiring this into the real ECU (e.g., the browser sending an actual brake frame on impact) is a reasonable future extension but was left out to avoid conflicting with the Brake ECU's own on/off state tracking.
- **The "idle creep" effect** (the road/obstacles drift slowly even at 0 broadcast speed) is a visualization-only convenience so the scene never looks frozen; it does not reflect a real vehicle behavior or a real CAN signal.
- **No real bus arbitration or priority-based collision handling** is modeled — this is a simplified simulation of the *architecture* (independent ECUs communicating over a shared bus), not a spec-compliant implementation of the CAN protocol's electrical/arbitration layer.
- **No DBC file or formal signal database** — CAN IDs and byte layouts are defined directly in `can_ids.h` and agreed upon manually across ECUs, rather than described in a shared, tooling-readable format.
- **The web bridge is a separate, optional layer** — the core embedded C system (Phases 1–10) is fully self-contained and runs correctly with or without the browser visualization present.

## Possible Extensions

- A real-time web dashboard (Python bridge + browser visualization) reading live CAN traffic
- DBC-based signal definitions instead of hardcoded byte offsets
- A basic rule-based intrusion/anomaly detector watching for abnormal frame timing or unexpected IDs