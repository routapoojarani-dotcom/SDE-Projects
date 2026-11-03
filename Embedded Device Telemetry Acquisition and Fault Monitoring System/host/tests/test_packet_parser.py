import json
import pytest
from telemetry_client.packet_parser import parse_packet

VALID_TELEMETRY = (
    '{"type":"telemetry","seq":1,"time_ms":100,"adc_mv":1650,'
    '"sensor_x":0.01,"sensor_y":-0.02,"sensor_z":0.98,'
    '"button":0,"status":"OK","faults":[]}'
)
VALID_RESPONSE = '{"type":"response","cmd":"PING","ok":true}'


def test_valid_telemetry_packet():
    pkt = parse_packet(VALID_TELEMETRY)
    assert pkt["type"] == "telemetry"
    assert pkt["seq"] == 1
    assert pkt["adc_mv"] == 1650
    assert pkt["faults"] == []
    assert pkt["status"] == "OK"


def test_invalid_json_raises():
    with pytest.raises(ValueError):
        parse_packet("not valid json{{{")


def test_missing_required_field_raises():
    data = json.loads(VALID_TELEMETRY)
    del data["seq"]
    with pytest.raises(ValueError, match="seq"):
        parse_packet(json.dumps(data))


def test_valid_response_packet():
    pkt = parse_packet(VALID_RESPONSE)
    assert pkt["type"] == "response"
    assert pkt["cmd"] == "PING"
    assert pkt["ok"] is True


def test_fault_packet_parses():
    line = (
        '{"type":"telemetry","seq":91,"time_ms":22300,"adc_mv":4095,'
        '"sensor_x":0.0,"sensor_y":0.0,"sensor_z":0.0,'
        '"button":1,"status":"FAULT","faults":["ADC_OUT_OF_RANGE","BUTTON_EVENT"]}'
    )
    pkt = parse_packet(line)
    assert pkt["status"] == "FAULT"
    assert "ADC_OUT_OF_RANGE" in pkt["faults"]
    assert "BUTTON_EVENT" in pkt["faults"]
