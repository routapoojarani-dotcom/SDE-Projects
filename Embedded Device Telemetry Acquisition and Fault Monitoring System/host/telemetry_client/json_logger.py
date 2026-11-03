import json
import datetime
from pathlib import Path


class JSONLTelemetryLogger:
    def __init__(self, output_path: str):
        Path(output_path).parent.mkdir(parents=True, exist_ok=True)
        self._file = open(output_path, "w", encoding="utf-8")

    def write_packet(self, packet: dict) -> None:
        enriched = dict(packet)
        enriched["pc_timestamp"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
        self._file.write(json.dumps(enriched, ensure_ascii=False) + "\n")

    def close(self) -> None:
        if not self._file.closed:
            self._file.close()

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()
