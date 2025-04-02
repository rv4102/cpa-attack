# process_readings.py
import sys
import pandas as pd
import numpy as np

def process_readings(input_file, output_file, num_plaintexts, S):
    # Load energy readings
    try:
        # Read CSV and convert energy_difference from hex to int if needed
        readings = pd.read_csv(input_file)
        
        # Check if the values appear to be in hex format
        sample_value = str(readings['energy_difference'].iloc[0]).lower()
        is_hex = sample_value.startswith('0x') or all(c in '0123456789abcdef' for c in sample_value)
        
        if is_hex:
            # Convert hex strings to integers
            readings['energy_difference'] = readings['energy_difference'].apply(
                lambda x: int(str(x), 16) if isinstance(x, str) else int(x)
            )
            print("Converted hex values to integers")
    except Exception as e:
        print(f"Error processing input file: {e}")
        sys.exit(1)
    
    # Reshape into matrix form (num_plaintexts x S)
    if len(readings) != num_plaintexts * S:
        print(f"Warning: Expected {num_plaintexts * S} measurements, but got {len(readings)}")
    
    # Create matrix of traces (one row per plaintext)
    traces = []
    for i in range(num_plaintexts):
        start_idx = i * S
        end_idx = min(start_idx + S, len(readings))
        
        if start_idx < len(readings):
            if end_idx <= len(readings):
                trace = readings['energy_difference'][start_idx:end_idx].values
                traces.append(','.join(map(str, trace)))
            else:
                print(f"Warning: Plaintext {i} has incomplete samples")
                # Use available samples
                trace = readings['energy_difference'][start_idx:].values
                # Pad with zeros if needed
                padding = [0] * (S - len(trace))
                full_trace = np.concatenate([trace, padding]) if padding else trace
                traces.append(','.join(map(str, full_trace)))
    
    # Write traces to output file
    with open(output_file, 'w') as f:
        f.write('\n'.join(traces))
    
    print(f"Processed {len(traces)} plaintexts with up to {S} samples each")
    print(f"Output written to {output_file}")

if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Usage: python process_readings.py <input_file> <output_file> <num_plaintexts> <S>")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    num_plaintexts = int(sys.argv[3])
    S = int(sys.argv[4])
    
    print(f"Processing {input_file}...")
    process_readings(input_file, output_file, num_plaintexts, S)