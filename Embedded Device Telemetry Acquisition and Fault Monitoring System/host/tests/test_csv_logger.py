import csv
import pytest
from telemetry_client.csv_logger import CSVTelemetryLogger

SAMPLE_PACKET = {
    "type": "telemetry",
    "seq": 1,
    "time_ms": 100,
    "adc_mv": 1650,
    "sensor_x": 0.01,
    "sensor_y": -0.02,
    "sensor_z": 0.98,
    "button": 0,
    "status": "OK",
    "faults": [],
}


def test_csv_file_created(tmp_path):
    path = str(tmp_path / "test.csv")
    with CSVTelemetryLogger(path) as logger:
        logger.write_packet(SAMPLE_PACKET)
    assert (tmp_path / "test.csv").exists()


def test_csv_header_correct(tmp_path):
    path = str(tmp_path / "test.csv")
    with CSVTelemetryLogger(path) as logger:
        logger.write_packet(SAMPLE_PACKET)
    with open(path, newline="") as f:
        reader = csv.reader(f)
        header = next(reader)
    assert header == [
        "pc_timestamp", "seq", "time_ms", "adc_mv",
        "sensor_x", "sensor_y", "sensor_z",
        "button", "status", "faults",
    ]


def test_telemetry_packet_writes_one_row(tmp_path):
    path = str(tmp_path / "test.csv")
    with CSVTelemetryLogger(path) as logger:
        logger.write_packet(SAMPLE_PACKET)
    with open(path, newline="") as f:
        rows = list(csv.reader(f))
    assert len(rows) == 2  # header + 1 data row


def test_faults_written_as_semicolon_string(tmp_path):
    path = str(tmp_path / "test.csv")
    packet = dict(SAMPLE_PACKET, faults=["ADC_OUT_OF_RANGE", "BUTTON_EVENT"])
    with CSVTelemetryLogger(path) as logger:
        logger.write_packet(packet)
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        row = next(reader)
    assert row["faults"] == "ADC_OUT_OF_RANGE;BUTTON_EVENT"
