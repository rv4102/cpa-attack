from scipy import stats
import pandas as pd
import sys


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
    print("TVLA Test Result: ", tvla(file1, file2))
