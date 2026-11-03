# Serial Protocol

## Transport

- **Interface**: USB CDC (virtual COM port) or hardware UART
- **Baud rate**: 115200 bps (fixed); telemetry period configurable via `SET_RATE`
- **Encoding**: UTF-8, newline-delimited (`\n`)
- **Direction**: bidirectional — host sends commands, device sends packets + responses

---

## Telemetry Packet Schema

The device streams one JSON object per line during active streaming.

```json
{
  "type":     "telemetry",
  "seq":      42,
  "time_ms":  12345,
  "adc_mv":   1820,
  "sensor_x": 0.02,
  "sensor_y": -0.01,
  "sensor_z": 0.98,
  "button":   0,
  "status":   "OK",
  "faults":   []
}
```

| Field | Type | Description |
|-------|------|-------------|
| `type` | string | Always `"telemetry"` |
| `seq` | integer | Monotonically increasing sequence number (starts at 1) |
| `time_ms` | integer | Device uptime in milliseconds |
| `adc_mv` | integer | ADC reading in millivolts (0–3300) |
| `sensor_x` | float | IMU X-axis (m/s²) or temperature (°C) |
| `sensor_y` | float | IMU Y-axis (m/s²) or pressure (hPa) |
| `sensor_z` | float | IMU Z-axis (m/s²) or humidity (%RH) |
| `button` | integer | Button state: `1` = pressed, `0` = released |
| `status` | string | `"OK"` or `"FAULT"` |
| `faults` | array | List of active fault name strings |

### Example — Normal Packet

```
{"type":"telemetry","seq":42,"time_ms":12345,"adc_mv":1820,"sensor_x":0.02,"sensor_y":-0.01,"sensor_z":0.98,"button":0,"status":"OK","faults":[]}
```

### Example — Fault Packet

```
{"type":"telemetry","seq":91,"time_ms":22300,"adc_mv":4095,"sensor_x":0.00,"sensor_y":0.00,"sensor_z":0.00,"button":1,"status":"FAULT","faults":["ADC_OUT_OF_RANGE","BUTTON_EVENT"]}
```

---

## Response Packet Schema

The device sends a response for every command received from the host.

```json
{
  "type": "response",
  "cmd":  "PING",
  "ok":   true
}
```

| Field | Type | Description |
|-------|------|-------------|
| `type` | string | Always `"response"` |
| `cmd` | string | The command this is a response to |
| `ok` | boolean | `true` on success, `false` on failure |
| additional fields | varies | Command-specific fields (see below) |

---

## Host Commands

Commands are plain ASCII strings terminated with `\n`.

| Command | Description |
|---------|-------------|
| `PING` | Check connectivity |
| `START` | Begin streaming telemetry packets |
| `STOP` | Halt telemetry streaming |
| `GET_STATUS` | Query current status and active faults |
| `GET_SENSOR` | Request a single sensor snapshot |
| `SET_RATE <ms>` | Set telemetry period in milliseconds (10–5000) |
| `CLEAR_FAULTS` | Clear all active fault flags |
| `RESET_STATS` | Reset packet and fault counters |

### Command Responses

**PING**
```json
{"type":"response","cmd":"PING","ok":true}
```

**START**
```json
{"type":"response","cmd":"START","ok":true,"streaming":true}
```

**STOP**
```json
{"type":"response","cmd":"STOP","ok":true,"streaming":false}
```

**GET_STATUS**
```json
{"type":"response","cmd":"GET_STATUS","ok":true,"status":"OK","faults":[]}
```

**SET_RATE 50**
```json
{"type":"response","cmd":"SET_RATE","ok":true,"period_ms":50}
```

**CLEAR_FAULTS**
```json
{"type":"response","cmd":"CLEAR_FAULTS","ok":true}
```

**RESET_STATS**
```json
{"type":"response","cmd":"RESET_STATS","ok":true}
```

**Invalid command**
```json
{"type":"response","cmd":"UNKNOWN","ok":false,"error":"INVALID_COMMAND"}
```

---

## Fault Types

| Fault | Trigger Condition |
|-------|-------------------|
| `ADC_OUT_OF_RANGE` | ADC reading < 100 mV or > 3200 mV |
| `SENSOR_TIMEOUT` | I2C sensor read returns failure |
| `BUTTON_EVENT` | Button is currently pressed |
| `HIGH_LATENCY` | Main loop took longer than telemetry period |
