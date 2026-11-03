import datetime
from .stats import (
    compute_packet_loss,
    compute_average_rate_hz,
    summarize_faults,
    summarize_adc_range,
)


def generate_report(packets: list, output_path: str) -> None:
    now = datetime.datetime.now(datetime.timezone.utc).isoformat()
    n = len(packets)

    if n >= 2:
        times = sorted(p["time_ms"] for p in packets)
        duration_s = (times[-1] - times[0]) / 1000.0
    else:
        duration_s = 0.0

    loss = compute_packet_loss(packets)
    rate = compute_average_rate_hz(packets)
    adc = summarize_adc_range(packets)
    faults = summarize_faults(packets)

    def fmt_adc(val):
        return f"{val:.1f}" if val is not None else "N/A"

    lines = [
        "# Telemetry Run Report",
        "",
        f"Generated: {now}",
        "",
        "## Summary",
        "",
        f"- Duration: {duration_s:.3f} s",
        f"- Packets received: {n}",
        f"- Estimated packet loss: {loss * 100:.2f}%",
        f"- Average packet rate: {rate:.2f} Hz",
        f"- Status: {'FAULT' if faults else 'OK'}",
        "",
        "## ADC Statistics",
        "",
        f"- Minimum ADC: {fmt_adc(adc['min'])} mV",
        f"- Maximum ADC: {fmt_adc(adc['max'])} mV",
        f"- Average ADC: {fmt_adc(adc['mean'])} mV",
        "",
        "## Fault Summary",
        "",
    ]

    if faults:
        lines.append("| Fault | Count |")
        lines.append("|-------|-------|")
        for fault, count in faults.items():
            lines.append(f"| {fault} | {count} |")
    else:
        lines.append("No faults recorded.")

    lines += ["", "## Notes", "", ""]

    with open(output_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))
