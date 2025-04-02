# process_readings.py
import sys
import pandas as pd
import numpy as np

def process_readings(input_file, output_file, num_plaintexts, S):
    readings = pd.read_csv(input_file)
    
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