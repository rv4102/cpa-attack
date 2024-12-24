import sys
import numpy as np
import pandas as pd


def main():
    if len(sys.argv) != 4:
        print("Usage: python cpa.py <traces_file> <hamm_file> <correct_key_byte>")
        sys.exit(1)

    traces_file = sys.argv[1]
    leakages_file = sys.argv[2]
    correct_key_byte = sys.argv[3]
    correct_key_byte = int(correct_key_byte, 16)

    T = pd.read_csv(traces_file, header=None)
    H = pd.read_csv(leakages_file, header=None)

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

    with open("results/guessing_entropy.txt", "a+") as f:
        f.write(str(correct_key_byte_index) + "\n")

    # print Hex value of the guessed key
    print(hex(guessed_key))


if __name__ == "__main__":
    main()
