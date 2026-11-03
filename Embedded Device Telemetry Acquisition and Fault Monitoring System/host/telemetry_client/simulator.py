import math


def generate_simulated_packet(seq: int, time_ms: int) -> dict:
    adc_mv = round(1650 + 400 * math.sin(seq * 0.15))
    sensor_x = round(0.02 + 0.05 * math.sin(seq * 0.3), 3)
    sensor_y = round(-0.01 + 0.05 * math.cos(seq * 0.3), 3)
    sensor_z = round(9.81 + 0.02 * math.sin(seq * 0.1), 3)
    button = 1 if (seq % 30 == 0 and seq > 0) else 0

    faults = []
    if seq % 20 == 0 and seq > 0:
        faults.append("ADC_OUT_OF_RANGE")
    if seq % 30 == 0 and seq > 0:
        faults.append("BUTTON_EVENT")

    status = "FAULT" if faults else "OK"

    return {
        "type": "telemetry",
        "seq": seq,
        "time_ms": time_ms,
        "adc_mv": adc_mv,
        "sensor_x": sensor_x,
        "sensor_y": sensor_y,
        "sensor_z": sensor_z,
        "button": button,
        "status": status,
        "faults": faults,
    }
