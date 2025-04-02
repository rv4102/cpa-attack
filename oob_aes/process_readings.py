# process_readings.py
import sys
import pandas as pd
import numpy as np

def process_readings(input_file, output_file, num_plaintexts, S):
    readings = pd.read_csv(input_file)
    
    # Reshape into matrix form (num_plaintexts x S)
    if len(readings) != num_plaintexts * S:
        print(f"Warning: Expected {num_plaintexts * S} measurements, but got {len(readings)}")
    
    readings = pd.DataFrame(readings['energy_difference'].to_numpy().reshape(num_plaintexts, S))
    readings.to_csv(output_file, header=False, index=False)

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