import json

REQUIRED_TELEMETRY_FIELDS = {"type", "seq", "time_ms", "adc_mv", "button", "status", "faults"}
REQUIRED_RESPONSE_FIELDS = {"type", "cmd", "ok"}


def parse_packet(line: str) -> dict:
    line = line.strip()
    if not line:
        raise ValueError("empty line")

    try:
        data = json.loads(line)
    except json.JSONDecodeError as e:
        raise ValueError(f"invalid JSON: {e}") from e

    pkt_type = data.get("type")
    if pkt_type is None:
        raise ValueError("missing field: type")

    if pkt_type == "telemetry":
        for field in REQUIRED_TELEMETRY_FIELDS:
            if field not in data:
                raise ValueError(f"missing field: {field}")
        data["faults"] = data["faults"] or []
    elif pkt_type == "response":
        for field in REQUIRED_RESPONSE_FIELDS:
            if field not in data:
                raise ValueError(f"missing field: {field}")
    else:
        raise ValueError(f"unknown packet type: {pkt_type}")

    return data
