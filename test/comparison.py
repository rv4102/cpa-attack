import sys
import pandas as pd

if __name__ == "__main__":
    traces_path_1 = sys.argv[1]
    traces_path_2 = sys.argv[2]

    t1 = pd.read_csv(traces_path_1, header=None).T
    t2 = pd.read_csv(traces_path_2, header=None).T

    # find co-variance between each corresponding row of the two traces
    cov = t1.cov(t2)

    # find average of the co-variance
    avg_cov = cov.mean()
    print(avg_cov)
