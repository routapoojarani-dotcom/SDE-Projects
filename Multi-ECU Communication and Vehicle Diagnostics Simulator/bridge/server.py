import can
import threading
import time
from flask import Flask, render_template
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

LABELS = {
    0x100: "rpm",
    0x101: "speed",
    0x102: "temp",
    0x103: "fuel",
    0x104: "brake",
    0x105: "turn_signal",
    0x106: "accelerator",
}

# Signals that update fast and should be throttled before hitting the browser
THROTTLE_MS = {
    "rpm": 500,
    "speed": 500,
    "temp": 500,
    "fuel": 500,
    # brake, turn_signal, accelerator are event-driven and rare — always emit immediately
}

last_emit_time = {}

def decode_value(can_id, data):
    if can_id == 0x100:
        return (data[0] << 8) | data[1]
    else:
        return data[0]

def should_emit(label):
    """Returns True if enough time has passed since this signal was last sent to the browser."""
    if label not in THROTTLE_MS:
        return True  # event-driven signals always pass through immediately

    now = time.time() * 1000
    last = last_emit_time.get(label, 0)
    if now - last >= THROTTLE_MS[label]:
        last_emit_time[label] = now
        return True
    return False

def can_listener_thread():
    bus = can.interface.Bus(channel="vcan0", interface="socketcan")
    print("CAN listener thread started, reading vcan0...")

    while True:
        msg = bus.recv()
        if msg is None:
            continue

        label = LABELS.get(msg.arbitration_id)
        if label is None:
            continue

        if not should_emit(label):
            continue  # skip this one — too soon since the last update for this signal

        value = decode_value(msg.arbitration_id, msg.data)
        socketio.emit("can_frame", {"signal": label, "value": value})

# A single shared bus instance for sending, separate from the listener's bus
send_bus = can.interface.Bus(channel="vcan0", interface="socketcan")

def send_frame(can_id, value):
    msg = can.Message(arbitration_id=can_id, data=[value], is_extended_id=False)
    send_bus.send(msg)

@app.route("/control/brake/<int:state>")
def control_brake(state):
    send_frame(0x104, state)
    return {"ok": True}

@app.route("/control/accelerator/<int:state>")
def control_accelerator(state):
    send_frame(0x106, state)
    return {"ok": True}

@app.route("/control/turn/<int:value>")
def control_turn(value):
    send_frame(0x105, value)
    return {"ok": True}

@app.route("/")
def index():
    return render_template("index.html")

if __name__ == "__main__":
    listener = threading.Thread(target=can_listener_thread, daemon=True)
    listener.start()

    print("Starting web server on http://0.0.0.0:5000")
    socketio.run(app, host="0.0.0.0", port=5000)