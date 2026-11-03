def compute_packet_loss(packets: list) -> float:
    if len(packets) < 2:
        return 0.0
    seqs = sorted(p["seq"] for p in packets)
    expected = seqs[-1] - seqs[0] + 1
    actual = len(packets)
    lost = expected - actual
    if lost <= 0:
        return 0.0
    return lost / expected


def compute_average_rate_hz(packets: list) -> float:
    if len(packets) < 2:
        return 0.0
    times = sorted(p["time_ms"] for p in packets)
    elapsed_ms = times[-1] - times[0]
    if elapsed_ms == 0:
        return 0.0
    return (len(packets) - 1) / (elapsed_ms / 1000.0)


def summarize_faults(packets: list) -> dict:
    counts: dict = {}
    for p in packets:
        for fault in (p.get("faults") or []):
            counts[fault] = counts.get(fault, 0) + 1
    return dict(sorted(counts.items(), key=lambda x: x[1], reverse=True))


def summarize_adc_range(packets: list) -> dict:
    vals = [p["adc_mv"] for p in packets if "adc_mv" in p]
    if not vals:
        return {"min": None, "max": None, "mean": None}
    return {
        "min": min(vals),
        "max": max(vals),
        "mean": sum(vals) / len(vals),
    }
