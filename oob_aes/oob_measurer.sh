#!/bin/bash

OUTPUT_FILE="results/readings.txt"
IA32_PMC0=0xC1
POLL_INTERVAL=0.0001  # 100µs

# Function to read MSR
read_msr() {
  local reg=$1
  output=$(peci_cmds RdIAMSR 0x0 $reg 2>/dev/null)
  if [ $? -ne 0 ]; then
    echo "Error executing peci_cmds" >&2
    echo "0"
    return
  fi

  msr_value=$(echo "$output" | sed -n 's/.*0x[0-9a-fA-F]\+ 0x\([0-9a-fA-F]\+\)/\1/p')
  if [ -z "$msr_value" ]; then
    echo "Error extracting MSR value from output: $output" >&2
    echo "0"
  else
    echo $((0x$msr_value))
  fi
}

# Function to read package energy using peci_cmds
read_pkg_energy() {
  # Extract the energy value (second hex number) from peci_cmds output
  # Format example: "cc:0x40 0x3ae62b96"
  output=$(peci_cmds RdPkgConfig 0x6 0x00 2>/dev/null)
  if [ $? -ne 0 ]; then
    echo "Error executing peci_cmds" >&2
    echo "0"
    return
  fi
  
  # Extract the second hex value (energy value)
  energy_val=$(echo "$output" | sed -n 's/.*0x[0-9a-fA-F]\+ 0x\([0-9a-fA-F]\+\)/\1/p')
  if [ -z "$energy_val" ]; then
    echo "Error extracting energy value from output: $output" >&2
    echo "0"
  else
    echo $((0x$energy_val))
  fi
}

echo "Started out-of-band measurement. Press Ctrl+C to stop..."

mkdir -p "results"
# Create output file with header
echo "measurement_id,energy_difference" > "$OUTPUT_FILE"

# Initialize variables
measuring=false
start_energy=0
end_energy=0
counter=0

# Trap Ctrl+C to clean up
trap 'echo -e "\nMeasurement stopped. Saved $counter measurements to $OUTPUT_FILE"; exit 0' INT TERM

# Main polling loop
while true; do
  # Read the LSB of IA32_PMC0
  pmc0_value=$(read_msr $IA32_PMC0)
  bit_set=$((pmc0_value & 1))
  
  if [ "$bit_set" -eq 1 ] && [ "$measuring" = false ]; then
    # Start of measurement
    measuring=true
    start_energy=$(read_pkg_energy)
    echo "Started measurement #$counter with energy: $start_energy"
  elif [ "$bit_set" -eq 0 ] && [ "$measuring" = true ]; then
    # End of measurement
    measuring=false
    end_energy=$(read_pkg_energy)
    energy_diff=$((end_energy - start_energy))
    
    # Write to the output file
    echo "$counter,$energy_diff" >> "$OUTPUT_FILE"
    
    # Increment counter and report progress
    counter=$((counter + 1))
    if [ $((counter % 10)) -eq 0 ]; then
      echo "Collected $counter measurements"
    fi
  fi
  
  # Sleep briefly to reduce CPU usage
  sleep $POLL_INTERVAL
done