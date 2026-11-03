#!/bin/bash

DURATION=${1:-1800}  # default 30 minutes, or pass seconds as first argument

echo "=== CAN Bus Simulator Stress Test ==="
echo "Duration: ${DURATION}s"
echo "Fault injection: ENABLED"
echo ""

# Make sure vcan0 exists (in case the VM was restarted)
ip link show vcan0 > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "vcan0 not found — creating it..."
    sudo modprobe vcan
    sudo ip link add dev vcan0 type vcan
    sudo ip link set up vcan0
fi

mkdir -p logs

echo "Starting ECUs..."
FAULT_INJECTION_ENABLED=1 ./build/engine_ecu > logs/stress_engine_stdout.txt 2>&1 &
ENGINE_PID=$!

./build/dashboard_ecu > logs/stress_dashboard_stdout.txt 2>&1 &
DASHBOARD_PID=$!

./build/lighting_ecu > logs/stress_lighting_stdout.txt 2>&1 &
LIGHTING_PID=$!

FAULT_INJECTION_ENABLED=1 ./build/brake_ecu > logs/stress_brake_stdout.txt 2>&1 &
BRAKE_PID=$!

FAULT_INJECTION_ENABLED=1 ./build/direction_ecu > logs/stress_direction_stdout.txt 2>&1 &
DIRECTION_PID=$!

echo "PIDs: engine=$ENGINE_PID dashboard=$DASHBOARD_PID lighting=$LIGHTING_PID brake=$BRAKE_PID direction=$DIRECTION_PID"
echo ""
echo "Recording memory usage every 30s to logs/stress_memory.csv"
echo "timestamp,engine_rss_kb,dashboard_rss_kb,lighting_rss_kb" > logs/stress_memory.csv

END_TIME=$(($(date +%s) + DURATION))

while [ $(date +%s) -lt $END_TIME ]; do
    TS=$(date +%s)
    ENGINE_RSS=$(ps -o rss= -p $ENGINE_PID 2>/dev/null | tr -d ' ')
    DASHBOARD_RSS=$(ps -o rss= -p $DASHBOARD_PID 2>/dev/null | tr -d ' ')
    LIGHTING_RSS=$(ps -o rss= -p $LIGHTING_PID 2>/dev/null | tr -d ' ')

    echo "$TS,$ENGINE_RSS,$DASHBOARD_RSS,$LIGHTING_RSS" >> logs/stress_memory.csv

    # Check none of the processes died unexpectedly
    for pid in $ENGINE_PID $DASHBOARD_PID $LIGHTING_PID $BRAKE_PID $DIRECTION_PID; do
        if ! kill -0 $pid 2>/dev/null; then
            echo "WARNING: process $pid died before test completed!"
        fi
    done

    sleep 30
done

echo ""
echo "Stress test duration complete. Stopping all ECUs..."
kill $ENGINE_PID $DASHBOARD_PID $LIGHTING_PID $BRAKE_PID $DIRECTION_PID 2>/dev/null

sleep 1
echo "Done. Check logs/stress_memory.csv and logs/*.log for results."