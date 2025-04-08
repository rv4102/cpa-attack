#!/bin/bash

OUTPUT_FILE="results/readings.csv"
IA32_PMC0=0xC1 # IA32_PMC0
PECI_MBX_INDEX_EPI=0x6 # Efficient Performance Indication
POLL_INTERVAL=0.0001  # 100µs


read_msr() {
  local reg=$1
  local max_retries=5  # Maximum number of retry attempts
  local retry_count=0
  local success=false
  
  while [ $retry_count -lt $max_retries ] && [ "$success" = false ]; do
    output=$(peci_cmds RdIAMSR 0x0 $reg 2>/dev/null)
    if [ $? -ne 0 ]; then
      echo "Error executing peci_cmds, retrying ($((retry_count+1))/$max_retries)..." >&2
      retry_count=$((retry_count+1))
      continue
    fi

    # Check if status code is 0x40 (success)
    status_code=$(echo "$output" | sed -n 's/cc:0x\([0-9a-fA-F]\+\).*/\1/p' | tr -d '[:space:]')
    if [ "$status_code" = "40" ]; then
      success=true
    else
      echo "Command returned status 0x$status_code instead of 0x40, retrying ($((retry_count+1))/$max_retries)..." >&2
      retry_count=$((retry_count+1))
      continue
    fi
    
    # Extract the second hex value (MSR value)
    # Format example: "cc:0x40 0x3ae62b96"
    msr_val=$(echo "$output" | sed -n 's/.*0x[0-9a-fA-F]\+ 0x\([0-9a-fA-F]\+\)/\1/p')
    if [ -z "$msr_val" ]; then
      echo "Error extracting energy value from output: $output" >&2
      echo "0"
      return
    else
      echo $((0x$msr_val))
      return
    fi
  done
  
  # If we've reached here, we've exhausted all retries
  if [ "$success" = false ]; then
    echo "Failed to read package energy after $max_retries attempts" >&2
    echo "0"
  fi
}


read_pkg_energy() {
  local reg=$1
  local max_retries=5
  local retry_count=0
  local success=false
  
  while [ $retry_count -lt $max_retries ] && [ "$success" = false ]; do
    output=$(peci_cmds RdPkgConfig 0x6 0x00 2>/dev/null)
    if [ $? -ne 0 ]; then
      echo "Error executing peci_cmds, retrying ($((retry_count+1))/$max_retries)..." >&2
      retry_count=$((retry_count+1))
      continue
    fi
    
    # Check if status code is 0x40 (success)
    status_code=$(echo "$output" | sed -n 's/cc:0x\([0-9a-fA-F]\+\).*/\1/p' | tr -d '[:space:]')
    if [ "$status_code" = "40" ]; then
      success=true
    else
      echo "Command returned status 0x$status_code instead of 0x40, retrying ($((retry_count+1))/$max_retries)..." >&2
      retry_count=$((retry_count+1))
      continue
    fi
    
    # Extract the second hex value (energy value)
    # Format example: "cc:0x40 0x3ae62b96"
    energy_val=$(echo "$output" | sed -n 's/.*0x[0-9a-fA-F]\+ 0x\([0-9a-fA-F]\+\)/\1/p')
    if [ -z "$energy_val" ]; then
      echo "Error extracting energy value from output: $output" >&2
      echo "0"
      return
    else
      echo $((0x$energy_val))
      return
    fi
  done
  
  # If we've reached here, we've exhausted all retries
  if [ "$success" = false ]; then
    echo "Failed to read package energy after $max_retries attempts" >&2
    echo "0"
  fi
}

echo "Started out-of-band measurement. Press Ctrl+C to stop..."

mkdir -p "results"

echo "measurement_id,energy_difference" > "$OUTPUT_FILE"

# Initialize variables
measuring=false
start_energy=0
end_energy=0
counter=0

trap 'echo -e "\nMeasurement stopped. Saved $counter measurements to $OUTPUT_FILE"; exit 0' INT TERM

# Main polling loop
while true; do
  # Read the LSB of IA32_PMC0
  pmc0_value=$(read_msr $IA32_PMC0)
  bit_set=$((pmc0_value & 1))
  
  if [ "$bit_set" -eq 1 ] && [ "$measuring" = false ]; then
    # Start of measurement
    measuring=true
    start_energy=$(read_pkg_energy $PECI_MBX_INDEX_EPI)
    echo "Started measurement #$counter with energy: $start_energy"
  elif [ "$bit_set" -eq 0 ] && [ "$measuring" = true ]; then
    # End of measurement
    measuring=false
    end_energy=$(read_pkg_energy $PECI_MBX_INDEX_EPI)
    energy_diff=$((end_energy - start_energy))
    
    # Write to the output file
    echo "$counter,$energy_diff" >> "$OUTPUT_FILE"
    
    # Increment counter and report progress
    counter=$((counter + 1))
  fi
  
  # Sleep briefly to reduce CPU usage
  sleep $POLL_INTERVAL
done