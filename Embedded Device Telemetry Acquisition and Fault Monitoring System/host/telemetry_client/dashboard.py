import sys
import time
from .stats import compute_packet_loss, compute_average_rate_hz


def run_dashboard(client, loggers=None, duration=None):
    packets = []
    start = time.time()
    print("Telemetry dashboard running. Press Ctrl+C to stop.\n")
    try:
        while True:
            if duration is not None and (time.time() - start) >= duration:
                break
            packet = client.read_packet()
            if packet is None:
                continue
            packets.append(packet)
            for logger in (loggers or []):
                logger.write_packet(packet)
            _render(packet, packets)
    except KeyboardInterrupt:
        print("\nStopped.")
    return packets


def _render(packet: dict, all_packets: list) -> None:
    recent = all_packets[-10:] if len(all_packets) >= 10 else all_packets
    rate = compute_average_rate_hz(recent) if len(recent) >= 2 else 0.0
    loss = compute_packet_loss(all_packets) * 100.0

    faults_str = ", ".join(packet.get("faults") or []) or "NONE"
    line = (
        f"\r[PKT#{packet.get('seq', '?'):>6}] "
        f"ADC:{packet.get('adc_mv', 0):>5}mV | "
        f"X:{packet.get('sensor_x', 0):>6.3f} "
        f"Y:{packet.get('sensor_y', 0):>6.3f} "
        f"Z:{packet.get('sensor_z', 0):>6.3f} | "
        f"Btn:{packet.get('button', 0)} | "
        f"{packet.get('status', '?'):>5} | "
        f"Faults:{faults_str} | "
        f"Rate:{rate:>5.1f}Hz | "
        f"Lost:{loss:>4.1f}%"
        + " " * 5
    )
    sys.stdout.write(line)
    sys.stdout.flush()
