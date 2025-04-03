import math
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


def plot_aes_trace(correlation_file: str = "results/correlation.csv", file_name: str = "plots/aes_trace.png"):
    df = pd.read_csv(correlation_file, header=None)

    # plot the first 6 rows of the dataframe in separate subplots
    fig, axs = plt.subplots(3, 2, figsize=(10, 10))
    for i in range(3):
        for j in range(2):
            axs[i, j].plot(df.iloc[i + j])

    fig.supxlabel("Trace index")
    fig.supylabel("MSR 0x612 Reading")
    plt.savefig(file_name)


def plot_correlation(correlation_file: str = "results/correlation.csv", file_name: str = "plots/correlation_plot.png"):
    df = pd.read_csv(correlation_file, header=None).T

    # for each row, find the min and max values
    mins = df.min(axis=1)
    maxs = df.max(axis=1)

    fig, ax = plt.subplots(figsize=(16, 5))
    ax.stem(range(256), mins, linefmt="b", markerfmt="bo", label="Group 1")
    ax.stem(range(256), maxs, linefmt="g", markerfmt="go", label="Group 2")

    ax.set_xlabel("All possible sub-key guesses")
    ax.set_ylabel("Correlation Value")
    plt.title("Correlation plot for byte " + str(0))
    plt.savefig(file_name)


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