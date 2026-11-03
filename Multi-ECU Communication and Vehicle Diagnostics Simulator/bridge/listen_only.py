import can

def decode_frame(msg):
    """Prints a human-readable line for a received CAN message."""
    can_id = msg.arbitration_id
    data = msg.data

    label = {
        0x100: "RPM",
        0x101: "Speed",
        0x102: "Engine Temp",
        0x103: "Fuel",
        0x104: "Brake",
        0x105: "Turn Signal",
        0x106: "Accelerator",
    }.get(can_id, f"Unknown (0x{can_id:X})")

    print(f"[{msg.timestamp:.3f}] ID=0x{can_id:X} ({label}) DATA={data.hex()}")

def main():
    # Connects to vcan0 using the "socketcan" backend — same interface your C code uses
    bus = can.interface.Bus(channel="vcan0", interface="socketcan")

    print("Listening on vcan0... (Ctrl+C to stop)")

    try:
        while True:
            msg = bus.recv()  # blocks until a frame arrives, just like your C can_receive()
            if msg is not None:
                decode_frame(msg)
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        bus.shutdown()

if __name__ == "__main__":
    main()