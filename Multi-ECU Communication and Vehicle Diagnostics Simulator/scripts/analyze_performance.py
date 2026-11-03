import re
import glob

def parse_log(path):
    """Reads one ECU's log file and returns a list of (timestamp_ms, direction, can_id) tuples."""
    frames = []
    with open(path) as f:
        for line in f:
            match = re.match(r'\[(\d+)\.(\d+)\] (TX|RX) ID=0x(\w+) DLC=(\d+)', line)
            if match:
                sec, ms, direction, can_id, dlc = match.groups()
                timestamp_ms = int(sec) * 1000 + int(ms)
                frames.append((timestamp_ms, direction, can_id, int(dlc)))
    return frames

def analyze(path):
    frames = parse_log(path)
    if not frames:
        print(f"{path}: no frames found")
        return

    tx_frames = [f for f in frames if f[1] == "TX"]
    rx_frames = [f for f in frames if f[1] == "RX"]

    start_time = frames[0][0]
    end_time = frames[-1][0]
    duration_sec = (end_time - start_time) / 1000

    print(f"--- {path} ---")
    print(f"Duration measured: {duration_sec:.1f}s")
    print(f"Total TX frames: {len(tx_frames)}")
    print(f"Total RX frames: {len(rx_frames)}")

    if duration_sec > 0:
        if tx_frames:
            print(f"TX throughput: {len(tx_frames) / duration_sec:.2f} frames/sec")
        if rx_frames:
            print(f"RX throughput: {len(rx_frames) / duration_sec:.2f} frames/sec")

    # Breakdown by CAN ID — shows which signal is actually dominating traffic
    id_counts = {}
    for f in tx_frames:
        can_id = f[2]
        id_counts[can_id] = id_counts.get(can_id, 0) + 1

    if id_counts:
        print("TX frames by CAN ID:")
        for can_id, count in sorted(id_counts.items()):
            print(f"  0x{can_id}: {count} frames")

    print()

# Analyze every ECU's log file automatically
for log_path in sorted(glob.glob("logs/*.log")):
    analyze(log_path)