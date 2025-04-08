#!/bin/bash

NUM_PLAINTEXTS=150
S=100
N=1000
READINGS_FILE="results/readings.csv"
TRACES_FILE="results/traces.csv"

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

pkg_power_limit() {
    local pkg_power_limit1=1300
    local enable_power_limit1=1
    local pkg_clamping_limit1=0
    local time_window1=15
    local pkg_power_limit2=1300
    local enable_power_limit2=1
    local pkg_clamping_limit2=0
    local time_window2=15
    local lock=0
    
    # Parse named arguments
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --pkg-power-limit1=*)
                pkg_power_limit1="${1#*=}"
                shift
                ;;
            --enable-power-limit1=*)
                enable_power_limit1="${1#*=}"
                shift
                ;;
            --pkg-clamping-limit1=*)
                pkg_clamping_limit1="${1#*=}"
                shift
                ;;
            --time-window1=*)
                time_window1="${1#*=}"
                shift
                ;;
            --pkg-power-limit2=*)
                pkg_power_limit2="${1#*=}"
                shift
                ;;
            --enable-power-limit2=*)
                enable_power_limit2="${1#*=}"
                shift
                ;;
            --pkg-clamping-limit2=*)
                pkg_clamping_limit2="${1#*=}"
                shift
                ;;
            --time-window2=*)
                time_window2="${1#*=}"
                shift
                ;;
            --lock=*)
                lock="${1#*=}"
                shift
                ;;
            *)
                echo "Unknown parameter: $1"
                return 1
                ;;
        esac
    done
    
    # Calculate the 64-bit value
    # Start with pkg_power_limit1 (bits 14:0)
    value=$(( pkg_power_limit1 & 0x7FFF ))
    
    # Add enable_power_limit1 (bit 15)
    value=$(( value | ((enable_power_limit1 & 0x1) << 15) ))
    
    # Add pkg_clamping_limit1 (bit 16)
    value=$(( value | ((pkg_clamping_limit1 & 0x1) << 16) ))
    
    # Add time_window1 (bits 23:17)
    value=$(( value | ((time_window1 & 0x7F) << 17) ))
    
    # Add pkg_power_limit2 (bits 46:32)
    value=$(( value | ((pkg_power_limit2 & 0x7FFF) << 32) ))
    
    # Add enable_power_limit2 (bit 47)
    value=$(( value | ((enable_power_limit2 & 0x1) << 47) ))
    
    # Add pkg_clamping_limit2 (bit 48)
    value=$(( value | ((pkg_clamping_limit2 & 0x1) << 48) ))

    # Add time_window2 (bits 55:49)
    value=$(( value | ((time_window2 & 0x7F) << 49) ))
    
    # Add lock (bit 63)
    value=$(( value | ((lock & 0x1) << 63) ))
    
    # Convert to hex for better readability
    hex_value=$(printf "0x%x" $value)
    
    echo "Writing to MSR 0x610: value=$hex_value"
    wrmsr 0x610 $value
}

echo "=========================================="
echo "   AES CPA Attack with MSR Synchronization"
echo "=========================================="

mkdir -p results

echo "[+] Setting power limits"
pkg_power_limit

# Set CPU affinity to ensure consistent behavior
echo "[+] Running AES encryption with PMC0 synchronization..."
sudo taskset -c 0-3 ./aes $NUM_PLAINTEXTS $S $N
echo "[+] Hamming weights saved to $OUTPUT_DIR/hamm<i>.csv"

echo "[+] Processing energy readings..."
python3 -c "import utils; utils.process_readings($READINGS_FILE, $TRACES_FILE, $NUM_PLAINTEXTS, $S)"
