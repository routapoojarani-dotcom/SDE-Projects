import sys
import serial
from .packet_parser import parse_packet


class SerialTelemetryClient:
    def __init__(self, port: str, baudrate: int = 115200, timeout: float = 1.0):
        self._port = port
        self._baudrate = baudrate
        self._timeout = timeout
        self._ser = None

    def connect(self) -> None:
        self._ser = serial.Serial(self._port, self._baudrate, timeout=self._timeout)

    def close(self) -> None:
        if self._ser and self._ser.is_open:
            self._ser.close()

    def send_command(self, command: str) -> None:
        self._ser.write((command.strip() + "\n").encode("utf-8"))
        self._ser.flush()

    def read_line(self) -> str:
        raw = self._ser.readline()
        return raw.decode("utf-8", errors="replace").strip()

    def read_packet(self):
        line = self.read_line()
        if not line:
            return None
        try:
            return parse_packet(line)
        except ValueError as e:
            print(f"\nWarning: bad packet ({e}): {line[:80]}", file=sys.stderr)
            return None

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, *args):
        self.close()
