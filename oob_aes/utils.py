import math
import numpy as np
import pandas as pd


def process_readings(input_file: str, output_file: str, num_plaintexts: int, S: int):
    num_plaintexts = int(num_plaintexts)
    S = int(S)
    readings = pd.read_csv(input_file)
    
    # Reshape into matrix form (num_plaintexts x S)
    if len(readings) != num_plaintexts * S:
        print(f"Warning: Expected {num_plaintexts * S} measurements, but got {len(readings)}")
    
    readings = pd.DataFrame(readings['energy_difference'].to_numpy().reshape(num_plaintexts, S))
    readings.to_csv(output_file, header=False, index=False)

    print(f"Output written to {output_file}")


def guessing_entropy(key_ranks_file: str):
    with open(key_ranks_file, "r") as f:
        key_ranks = f.readlines()

    guessing_entropy = sum([math.log2(float(x)) for x in key_ranks])
    print(f"Guessing Entropy: {guessing_entropy}")


def cpa(traces_file: str, hamming_file: str, key_ranks_file: str, correct_key_byte: int):
    correct_key_byte = int(correct_key_byte, 16)
    T = pd.read_csv(traces_file, header=None)
    H = pd.read_csv(hamming_file, header=None)

    coeff_shape = [T.shape[1], H.shape[1]]

    # Column-wise Pearson's correlation coefficient matrix
    correlation_matrix = np.corrcoef(T.values, H.values, rowvar=False)
    correlation_matrix = correlation_matrix[: coeff_shape[0], coeff_shape[0] :]
    correlation_df = pd.DataFrame(
        correlation_matrix, index=T.columns, columns=H.columns
    )
    correlation_df.to_csv("results/correlation.csv", header=False, index=False)
    max_index = correlation_df.stack().idxmax()
    guessed_key = max_index[1]

    max_values = correlation_df.max()
    max_values = max_values.reset_index()
    max_values.columns = ["key_byte", "correlation"]
    max_values = max_values.sort_values(by="correlation", ascending=False)["key_byte"]
    max_values = max_values.reset_index(drop=True)
    correct_key_byte_index = max_values[max_values == correct_key_byte].index[0]
    correct_key_byte_index += 1  # 1-indexed

    with open(key_ranks_file, "a+") as f:
        f.write(str(correct_key_byte_index) + "\n")

    # print Hex value of the guessed key
    print(f"{guessed_key:02x}")