import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results/correlation.csv", header=None)

# plot the first 6 rows of the dataframe in separate subplots
fig, axs = plt.subplots(3, 2, figsize=(10, 10))
for i in range(3):
    for j in range(2):
        axs[i, j].plot(df.iloc[i + j])

fig.supxlabel("Trace index")
fig.supylabel("MSR 0x612 Reading")
plt.savefig("plots/aes_trace.png")
