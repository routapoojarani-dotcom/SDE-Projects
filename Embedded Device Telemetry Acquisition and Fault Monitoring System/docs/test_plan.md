# Test Plan

## Unit Tests (Python)

Run all Python tests with:

```bash
cd <repo-root>
pip install -r host/requirements.txt
pytest host/tests -v
```

### test_packet_parser.py

| # | Test Case | Expected Result |
|---|-----------|-----------------|
| 1 | Valid telemetry packet | Returns dict, `type == "telemetry"`, all fields present |
| 2 | Invalid JSON string | Raises `ValueError` |
| 3 | Missing required field (`seq`) | Raises `ValueError` matching field name |
| 4 | Valid response packet | Returns dict, `type == "response"` |
| 5 | Fault packet with non-empty faults | Parses correctly, faults list populated |

### test_csv_logger.py

| # | Test Case | Expected Result |
|---|-----------|-----------------|
| 1 | Write a packet | CSV file created at output path |
| 2 | Read header row | Matches required column order |
| 3 | Write one packet | File has exactly 2 rows (header + 1 data) |
| 4 | Write packet with faults | `faults` column = `"A;B"` (semicolon-separated) |

### test_stats.py

| # | Test Case | Expected Result |
|---|-----------|-----------------|
| 1 | Continuous seq 0–4 | `compute_packet_loss` returns `0.0` |
| 2 | Seq gap: [0, 2] | `compute_packet_loss` returns `1/3` ≈ 0.333 |
| 3 | 11 packets over 1000 ms | `compute_average_rate_hz` returns `10.0` |
| 4 | Packets with mixed faults | `summarize_faults` counts each fault correctly |
| 5 | Known ADC values | `summarize_adc_range` returns correct min/max/mean |

### test_report_generator.py

| # | Test Case | Expected Result |
|---|-----------|-----------------|
| 1 | `generate_report` called | Markdown file created |
| 2 | 10-packet report | "Packets received: 10" appears in content |
| 3 | Any report | "packet loss" appears (case-insensitive) |
| 4 | 40-packet report (includes faults at seq 20) | "Fault Summary" section present |
| 5 | Any report | "ADC Statistics" section present |

---

## Hardware Validation Steps

### Pre-flight Check

1. Flash firmware to STM32 Nucleo board.
2. Connect via USB; verify COM port appears (Device Manager on Windows).
3. Open a serial terminal (115200 8N1).
4. Send `PING` — expect `{"type":"response","cmd":"PING","ok":true}`.

### Streaming Test

1. Send `START`.
2. Verify JSON telemetry packets stream at ~10 Hz (100 ms period).
3. Rotate potentiometer — verify `adc_mv` changes.
4. Press user button — verify `button: 1` and `BUTTON_EVENT` in faults.
5. Send `STOP` — verify streaming halts.

### Fault Tests

| Scenario | Action | Expected |
|----------|--------|----------|
| ADC low | Pull potentiometer fully to GND | `adc_mv < 100`, `ADC_OUT_OF_RANGE` in faults, red LED on |
| ADC high | Pull potentiometer to 3.3 V | `adc_mv > 3200`, `ADC_OUT_OF_RANGE` in faults |
| I2C fail | Disconnect I2C sensor cable | `SENSOR_TIMEOUT` in faults |
| Button | Press B1 USER button | `button: 1`, `BUTTON_EVENT` in faults |
| Clear | Send `CLEAR_FAULTS` | Faults list empty, green LED on |

### Rate Change Test

1. Send `SET_RATE 500` — verify packets arrive every ~500 ms.
2. Send `SET_RATE 50` — verify packets arrive every ~50 ms.
3. Send `SET_RATE 0` — expect error response.

### Python Host Integration Test

1. Run: `python -m telemetry_client.cli run --port COM5 --baud 115200 --csv data/run.csv --jsonl data/run.jsonl`
2. Let it run for 30 seconds.
3. Press Ctrl+C.
4. Verify CSV and JSONL files contain data.
5. Run: `python -m telemetry_client.cli report --input data/run.jsonl --output reports/run.md`
6. Verify report contains correct packet count, rate, and ADC stats.

---

## Acceptance Criteria

- [ ] Firmware streams newline-delimited JSON telemetry at 100 ms
- [ ] Telemetry includes seq, time_ms, adc_mv, sensor_x/y/z, button, status, faults
- [ ] All 8 commands (PING/START/STOP/GET_STATUS/GET_SENSOR/SET_RATE/CLEAR_FAULTS/RESET_STATS) respond correctly
- [ ] Python host parses valid packets and rejects malformed ones without crashing
- [ ] CSV and JSONL logs are created and contain correct data
- [ ] `pytest host/tests` passes all 19 test cases
- [ ] Simulated mode (`cli simulate`) runs without hardware
- [ ] Markdown report generated with correct stats
- [ ] README explains how to run the project in under 60 seconds of reading
