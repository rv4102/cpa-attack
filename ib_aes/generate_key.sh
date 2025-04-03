#!/bin/bash

CORRECT_KEY="00000000000000000000000000000000"
RECOVERED_KEY=""
NUM_PLAINTEXTS=1000
S=10000
N=50000

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

# Print banner
echo "=========================================="
echo "   AES CPA Attack Automation Script       "
echo "=========================================="

echo "[+] Setting power limits"
pkg_power_limit

# Generate traces and plaintexts
echo "[+] Generating power traces and plaintexts..."
sudo taskset -c 0-3 ./aes $NUM_PLAINTEXTS $S $N
echo "[+] Traces saved to results/traces.csv"
echo "[+] Plaintexts saved to results/plaintexts.txt"
# generate hamming weights
echo "[+] Generating hamming weight model..."
./hamming $NUM_PLAINTEXTS
echo "[+] Hamming weights saved to results/hamm<i>.csv"

# Create directory for results if it doesn't exist
mkdir -p results

# Reset key_ranks.txt
truncate -s 0 results/key_ranks.txt

# Process each byte position
for i in {0..15}; do
    echo "[+] Processing key byte position $i..."
    
    # Extract the correct key byte for this position (for evaluation)
    CORRECT_BYTE=$(echo $CORRECT_KEY | cut -c $((2*i+1))-$((2*i+2)))
    
    # Run CPA analysis for this byte position
    echo "    Running CPA analysis..."
    BYTE_RESULT=$(python3 -c "import utils; utils.cpa(results/traces.csv, results/hamm${i}.csv, $CORRECT_BYTE)"
    
    # Append to the recovered key
    RECOVERED_KEY="${RECOVERED_KEY}${BYTE_RESULT}"
    
    echo "    Key byte $i: $BYTE_RESULT"
done

# Save the full recovered key
echo "[+] Full recovered key: $RECOVERED_KEY"

# Compile guessing entropy results
echo "[+] Guessing entropy results..."
python3 -c "import utils; utils.guessing_entropy(results/key_ranks.txt)"

# Validate the recovered key if the correct key is known
if [ "$RECOVERED_KEY" == "$CORRECT_KEY" ]; then
    echo "[+] SUCCESS: Recovered key matches the correct key!"
else
    echo "[!] WARNING: Recovered key does not match the correct key"
    echo "    Correct:  $CORRECT_KEY"
    echo "    Recovered: $RECOVERED_KEY"
    
    # Count how many bytes are correct
    CORRECT_BYTES=0
    for i in {0..15}; do
        CORRECT_BYTE=$(echo $CORRECT_KEY | cut -c $((2*i+1))-$((2*i+2)))
        RECOVERED_BYTE=$(echo $RECOVERED_KEY | cut -c $((2*i+1))-$((2*i+2)))
        if [ "$CORRECT_BYTE" == "$RECOVERED_BYTE" ]; then
            CORRECT_BYTES=$((CORRECT_BYTES+1))
        fi
    done
    
    echo "    Correctly recovered $CORRECT_BYTES out of 16 key bytes"
fi

echo "[+] Attack completed. Results saved in the 'results' directory."
echo "=========================================="
