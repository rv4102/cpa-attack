from scipy import stats
import pandas as pd
import sys
import matplotlib.pyplot as plt


def tvla(data1, data2, alpha=4.5):
    t_stat, p_val = stats.ttest_ind(data1, data2, equal_var=False)
    print("T-Statisitc: ", t_stat, "; P-Value: ", p_val)

    if abs(t_stat) > alpha:
        # print("Reject Null Hypothesis: Significant side-channel leakage detected")
        return 1
    else:
        # print("Accept Null Hypothesis: No significant side-channel leakage detected")
        return 0


if __name__ == "__main__":
    file1 = pd.read_csv(sys.argv[1])
    file2 = pd.read_csv(sys.argv[2])

    print("TVLA Test Result: ", tvla(file1.values, file2.values))
    # create plot comparing file1 and file2
    plt.plot(file1, label=sys.argv[1])
    plt.plot(file2, label=sys.argv[2])
    plt.legend()
    plt.savefig("plot_" + sys.argv[3] + ".png")
