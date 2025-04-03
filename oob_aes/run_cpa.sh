#!/bin/bash

CORRECT_KEY="00000000000000000000000000000000"
OUTPUT_DIR="results"
READINGS_FILE="results/readings.csv"
TRACES_FILE="results/traces.csv"
KEY_RANKS_FILE="results/key_ranks.txt"
NUM_PLAINTEXTS=50
S=50

echo "[+] Processing energy readings..."
python3 -c "import utils; utils.process_readings($READINGS_FILE, $TRACES_FILE, $NUM_PLAINTEXTS, $S)"

# Reset key_ranks.txt
truncate -s 0 $KEY_RANKS_FILE

# Process each byte position
RECOVERED_KEY=""
for i in {0..15}; do
    echo "[+] Processing key byte position $i..."
    
    # Extract the correct key byte for this position (for evaluation)
    CORRECT_BYTE=$(echo $CORRECT_KEY | cut -c $((2*i+1))-$((2*i+2)))
    
    # Run CPA analysis for this byte position
    echo "    Running CPA analysis..."
    BYTE_RESULT=$(python3 -c "import utils; utils.cpa($TRACES_FILE, '$OUTPUT_DIR/hamm${i}.csv', $CORRECT_BYTE)"
    
    # Append to the recovered key
    RECOVERED_KEY="${RECOVERED_KEY}${BYTE_RESULT}"
    
    echo "    Key byte $i: $BYTE_RESULT"
done

# Save the full recovered key
echo "[+] Full recovered key: $RECOVERED_KEY"

# Compile guessing entropy results
echo "[+] Guessing entropy results..."
python3 -c "import utils; utils.guessing_entropy($KEY_RANKS_FILE)"

# Validate the recovered key
if [ "$RECOVERED_KEY" == "$CORRECT_KEY" ]; then
    echo "[+] SUCCESS: Recovered key matches th\e correct key!"
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

echo "[+] Attack completed. Results saved in the '$OUTPUT_DIR' directory."
echo "=========================================="
