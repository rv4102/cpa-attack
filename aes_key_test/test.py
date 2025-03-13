from scipy import stats
import pandas as pd
import matplotlib.pyplot as plt
import sys
import numpy as np


def filter(readings: pd.DataFrame):
    readings_column = readings.iloc[:, 0]

    # Calculate Q1, Q3, and IQR
    Q1 = np.percentile(readings_column, 25)
    Q3 = np.percentile(readings_column, 75)
    IQR = Q3 - Q1

    # Define lower and upper bounds for non-outliers
    lower_bound = Q1 - 1.5 * IQR
    upper_bound = Q3 + 1.5 * IQR

    filtered_readings = readings_column[
        (readings_column >= lower_bound) & (readings_column <= upper_bound)
    ]

    filtered_readings = filtered_readings.reset_index(drop=True)
    return filtered_readings


def tvla(data1, data2, alpha=4.5):
    t_stat, p_val = stats.ttest_ind(data1, data2, equal_var=False)
    print("T-Statisitc: ", t_stat, "; P-Value: ", p_val)

    if abs(t_stat) > alpha:
        # print("Reject Null Hypothesis: Significant side-channel leakage detected")
        return 1
    else:
        # print("Accept Null Hypothesis: No significant side-channel leakage detected")
        return 0


def time_series_plot(data1: pd.DataFrame, data2: pd.DataFrame, name: str) -> None:
    plt.plot(data1, label=sys.argv[1])
    plt.plot(data2, label=sys.argv[2])
    plt.legend()
    plt.xlabel("Reading Number")
    plt.ylabel("Reading Value")
    plt.title("Time Series Plot of Readings")
    plt.savefig("data/plot_" + name + ".png")


def freq_plot(data1: pd.DataFrame, data2: pd.DataFrame, name: str) -> None:
    plt.hist(data1, bins=1000, edgecolor="blue")
    plt.hist(data2, bins=1000, edgecolor="red")
    plt.legend(["Zero", "Full"])
    plt.xlabel("Reading Value")
    plt.ylabel("Frequency")
    plt.title("Frequency Plot of Readings")
    plt.savefig("data/hist_" + name + ".png")


if __name__ == "__main__":
    file1 = pd.read_csv(sys.argv[1], header=None)
    file2 = pd.read_csv(sys.argv[2], header=None)

    # filter out the first row
    file1 = file1.iloc[1:]
    file2 = file2.iloc[1:]

    print("TVLA Test Result: ", tvla(file1.values, file2.values))
    time_series_plot(file1, file2, sys.argv[3])

    file1 = filter(file1)
    file2 = filter(file2)
    freq_plot(file1, file2, sys.argv[3])
