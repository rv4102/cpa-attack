#!/bin/bash

POWER=1300
ENABLE1=1
ENABLE2=1
CLAMP1=0
CLAMP2=0
TIME_WINDOW1=15
TIME_WINDOW2=15
NUM_PLAINTEXTS=100
S=100  # Reduced number of samples per plaintext
N=1000  # Reduced number of AES operations per sample
CPU_ID=0

# Stop on errors
set -e

if [ "$EUID" -ne 0 ]; then
    echo "This script must be run as root to access MSRs."
    echo "Please run with: sudo $0"
    exit 1
fi

# Check if MSR module is loaded
if ! lsmod | grep -q "^msr "; then
    echo "Loading MSR module..."
    modprobe msr
    if [ $? -ne 0 ]; then
        echo "Failed to load MSR module. Are you running a supported kernel?"
        exit 1
    fi
fi

echo "=========================================="
echo "   AES CPA Attack with MSR Synchronization"
echo "=========================================="

echo "[+] Setting power limits"
./controller $POWER $POWER $ENABLE1 $ENABLE2 $CLAMP1 $CLAMP2 $TIME_WINDOW1 $TIME_WINDOW2

# Set CPU affinity to ensure consistent behavior
echo "[+] Running AES encryption with PMC0 synchronization..."
sudo taskset -c $CPU_ID ./aes $NUM_PLAINTEXTS $S $N