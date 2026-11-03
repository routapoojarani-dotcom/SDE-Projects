import argparse
import sys
import time

from .packet_parser import parse_packet
from .csv_logger import CSVTelemetryLogger
from .json_logger import JSONLTelemetryLogger
from .report_generator import generate_report
from .simulator import generate_simulated_packet
from .dashboard import _render


def cmd_run(args):
    from .serial_client import SerialTelemetryClient
    from .dashboard import run_dashboard

    client = SerialTelemetryClient(args.port, args.baud)
    loggers = []
    if args.csv:
        loggers.append(CSVTelemetryLogger(args.csv))
    if args.jsonl:
        loggers.append(JSONLTelemetryLogger(args.jsonl))

    try:
        client.connect()
        client.send_command("START")
        run_dashboard(client, loggers)
    finally:
        try:
            client.send_command("STOP")
        except Exception:
            pass
        client.close()
        for logger in loggers:
            logger.close()


def cmd_command(args):
    from .serial_client import SerialTelemetryClient

    with SerialTelemetryClient(args.port) as client:
        client.send_command(args.cmd)
        response = client.read_line()
        print(response)


def cmd_report(args):
    packets = []
    with open(args.input, "r", encoding="utf-8") as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            try:
                packet = parse_packet(line)
                if packet.get("type") == "telemetry":
                    packets.append(packet)
            except ValueError as e:
                print(f"Warning: skipping line {line_num}: {e}", file=sys.stderr)

    generate_report(packets, args.output)
    print(f"Report written to {args.output} ({len(packets)} packets)")


def cmd_simulate(args):
    loggers = []
    if args.csv:
        loggers.append(CSVTelemetryLogger(args.csv))
    if args.jsonl:
        loggers.append(JSONLTelemetryLogger(args.jsonl))

    duration = args.duration
    seq = 0
    time_ms = 0
    start = time.time()
    packets = []

    print(f"Simulating {duration}s of telemetry. Press Ctrl+C to stop early.")
    try:
        while (time.time() - start) < duration:
            packet = generate_simulated_packet(seq, time_ms)
            packets.append(packet)
            for logger in loggers:
                logger.write_packet(packet)
            _render(packet, packets)
            seq += 1
            time_ms += 100
            time.sleep(0.100)
    except KeyboardInterrupt:
        pass
    finally:
        print(f"\nSimulated {seq} packets.")
        for logger in loggers:
            logger.close()


def main():
    parser = argparse.ArgumentParser(
        prog="telemetry_client.cli",
        description="Embedded Telemetry Logger — host-side CLI",
    )
    sub = parser.add_subparsers(dest="command", required=True)

    p_run = sub.add_parser("run", help="Connect to device and log live telemetry")
    p_run.add_argument("--port", required=True, help="Serial port (e.g. COM5 or /dev/ttyUSB0)")
    p_run.add_argument("--baud", type=int, default=115200)
    p_run.add_argument("--csv", help="CSV log output path")
    p_run.add_argument("--jsonl", help="JSONL log output path")

    p_cmd = sub.add_parser("command", help="Send a single command to the device")
    p_cmd.add_argument("--port", required=True)
    p_cmd.add_argument("--cmd", required=True, help="Command string (e.g. PING)")

    p_rep = sub.add_parser("report", help="Generate Markdown report from a JSONL log")
    p_rep.add_argument("--input", required=True, help="Path to .jsonl log file")
    p_rep.add_argument("--output", required=True, help="Path for Markdown report output")

    p_sim = sub.add_parser("simulate", help="Generate simulated telemetry without hardware")
    p_sim.add_argument("--csv", help="CSV output path")
    p_sim.add_argument("--jsonl", help="JSONL output path")
    p_sim.add_argument("--duration", type=float, default=10.0, help="Simulation duration (seconds)")

    args = parser.parse_args()
    dispatch = {
        "run": cmd_run,
        "command": cmd_command,
        "report": cmd_report,
        "simulate": cmd_simulate,
    }
    dispatch[args.command](args)


if __name__ == "__main__":
    main()
