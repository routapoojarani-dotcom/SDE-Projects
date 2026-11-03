#!/bin/bash

echo "Setting up vcan0..."

sudo modprobe vcan

if ip link show vcan0 > /dev/null 2>&1; then
    echo "vcan0 already exists."
else
    sudo ip link add dev vcan0 type vcan
fi

sudo ip link set up vcan0

echo "Done. Status:"
ip link show vcan0