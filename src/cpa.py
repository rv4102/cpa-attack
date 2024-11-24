import sys
import numpy as np
import pandas as pd

def main():
    if len(sys.argv) != 3:
        print("Usage: python cpa.py <traces_file> <hamm_file>")
        sys.exit(1)

    traces_file = sys.argv[1]
    leakages_file = sys.argv[2]

    H = pd.read_csv(leakages_file, header=None).T
    T = pd.read_csv(traces_file, header=None).T 

    T_normalized = (T - T.mean()) / T.std()
    H_normalized = (H - H.mean()) / H.std()

    # Compute the correlation matrix
    correlation_matrix = np.dot(T_normalized.values, H_normalized.values.T) / T.shape[1]
    correlation_df = pd.DataFrame(correlation_matrix, index=T.index, columns=H.index)
    max_index = correlation_df.stack().idxmax()

    guessed_key = max_index[1]
    # print Hex value of the guessed key
    print(hex(guessed_key))


if __name__ == "__main__":
    main()
