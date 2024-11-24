import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results/correlation.csv", header=None).T

# for each row, find the min and max values
mins = df.min(axis=1)
maxs = df.max(axis=1)

fig, ax = plt.subplots(figsize=(16, 5))
ax.stem(range(256), mins, linefmt="b", markerfmt="bo", label="Group 1")
ax.stem(range(256), maxs, linefmt="g", markerfmt="go", label="Group 2")

ax.set_xlabel("All possible sub-key guesses")
ax.set_ylabel("Correlation Value")
plt.title("Correlation plot for byte " + str(0))
plt.savefig("plots/correlation_plot.png")
