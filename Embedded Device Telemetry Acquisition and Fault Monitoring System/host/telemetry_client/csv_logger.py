import csv
import datetime
from pathlib import Path

COLUMNS = [
    "pc_timestamp", "seq", "time_ms", "adc_mv",
    "sensor_x", "sensor_y", "sensor_z",
    "button", "status", "faults",
]


class CSVTelemetryLogger:
    def __init__(self, output_path: str):
        Path(output_path).parent.mkdir(parents=True, exist_ok=True)
        self._file = open(output_path, "w", newline="", encoding="utf-8")
        self._writer = csv.DictWriter(self._file, fieldnames=COLUMNS, extrasaction="ignore")
        self._writer.writeheader()

    def write_packet(self, packet: dict) -> None:
        row = {
            "pc_timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat(),
            "seq":          packet.get("seq", ""),
            "time_ms":      packet.get("time_ms", ""),
            "adc_mv":       packet.get("adc_mv", ""),
            "sensor_x":     packet.get("sensor_x", ""),
            "sensor_y":     packet.get("sensor_y", ""),
            "sensor_z":     packet.get("sensor_z", ""),
            "button":       packet.get("button", ""),
            "status":       packet.get("status", ""),
            "faults":       ";".join(packet.get("faults") or []),
        }
        self._writer.writerow(row)

    def close(self) -> None:
        if not self._file.closed:
            self._file.close()

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()
