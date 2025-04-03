#!/bin/bash

CORRECT_KEY="00000000000000000000000000000000"
OUTPUT_DIR="results"
NUM_PLAINTEXTS=50
S=50

# Process the energy readings to create trace data
echo "[+] Processing energy readings..."
python3 process_readings.py $OUTPUT_DIR/readings.csv $OUTPUT_DIR/traces.csv $NUM_PLAINTEXTS $S

# Generate hamming weights
echo "[+] Generating hamming weight model..."
./hamming $NUM_PLAINTEXTS
echo "[+] Hamming weights saved to $OUTPUT_DIR/hamm<i>.csv"

# Reset key_ranks.txt
truncate -s 0 $OUTPUT_DIR/key_ranks.txt

# Process each byte position
RECOVERED_KEY=""
for i in {0..15}; do
    echo "[+] Processing key byte position $i..."
    
    # Extract the correct key byte for this position (for evaluation)
    CORRECT_BYTE=$(echo $CORRECT_KEY | cut -c $((2*i+1))-$((2*i+2)))
    
    # Run CPA analysis for this byte position
    echo "    Running CPA analysis..."
    BYTE_RESULT=$(python3 cpa.py $OUTPUT_DIR/traces.csv $OUTPUT_DIR/hamm${i}.csv $CORRECT_BYTE)
    
    # Append to the recovered key
    RECOVERED_KEY="${RECOVERED_KEY}${BYTE_RESULT}"
    
    echo "    Key byte $i: $BYTE_RESULT"
done

# Save the full recovered key
echo "[+] Full recovered key: $RECOVERED_KEY"

# Compile guessing entropy results
echo "[+] Guessing entropy results..."
python3 guessing_entropy.py

# Validate the recovered key
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

echo "[+] Attack completed. Results saved in the '$OUTPUT_DIR' directory."
echo "=========================================="
