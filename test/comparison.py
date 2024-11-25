import sys
import pandas as pd

if __name__ == "__main__":
    traces_path_1 = sys.argv[1]
    traces_path_2 = sys.argv[2]

    t1 = pd.read_csv(traces_path_1, header=None).T
    t2 = pd.read_csv(traces_path_2, header=None).T

    t1 = (t1 - t1.mean(axis=1)) / (t1.std())
    t2 = (t2 - t2.mean(axis=1)) / (t2.std())

    correlations = []
    avg_corr = 0.0
    x = t1.shape[1]
    for i in range(x):
        correlations.append(t1[i].corr(t2[i]))
        avg_corr += correlations[i]

    avg_corr = avg_corr / x
    print(avg_corr)
