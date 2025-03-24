#!/bin/bash

POWER=1300
ENABLE1=1
ENABLE2=1
CLAMP1=0
CLAMP2=0
TIME_WINDOW1=15
TIME_WINDOW2=15
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

# Print banner
echo "=========================================="
echo "   AES CPA Attack Automation Script       "
echo "=========================================="

echo "[+] Setting power limits"
#./controller $POWER $POWER $ENABLE1 $ENABLE2 $CLAMP1 $CLAMP2 $TIME_WINDOW1 $TIME_WINDOW2

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
    BYTE_RESULT=$(python3 cpa.py results/traces.csv results/hamm${i}.csv $CORRECT_BYTE)
    
    # Append to the recovered key
    RECOVERED_KEY="${RECOVERED_KEY}${BYTE_RESULT}"
    
    echo "    Key byte $i: $BYTE_RESULT"
done

# Save the full recovered key
echo "[+] Full recovered key: $RECOVERED_KEY"

# Compile guessing entropy results
echo "[+] Guessing entropy results..."
python3 guessing_entropy.py

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
