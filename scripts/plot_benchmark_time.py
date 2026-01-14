import pandas as pd
import matplotlib.pyplot as plt

# -----------------------------
# Load data
# -----------------------------
csv_path = "results/benchmark_time.csv"
df = pd.read_csv(csv_path)

nodes = df["nodes"]

# Algorithms to plot
algos = [
    ("baseline", "Baseline"),
    ("greedy", "Greedy"),
    ("local_search", "Local Search"),
    ("hill_climbing", "Hill Climbing"),
    ("lp_tkr", "LP-TKR"),
    ("lp_irp", "LP-IRP"),
]

# -----------------------------
# Plot
# -----------------------------
plt.figure(figsize=(8, 6))

for col, label in algos:
    plt.plot(
        nodes,
        df[col],
        marker="o",
        linewidth=2,
        label=label
    )

# Log scales
plt.xscale("log", base=2)     # nodes: 64,128,256,...
plt.yscale("log")             # time in seconds

plt.xlabel("Number of Nodes (log scale)")
plt.ylabel("Wall-clock Time (seconds, log scale)")
plt.title("Vaccination Algorithms Time Complexity (Log–Log)")

plt.grid(True, which="both", linestyle="--", alpha=0.6)
plt.legend()

# -----------------------------
# Save & show
# -----------------------------
plt.tight_layout()
plt.savefig("results/benchmark_time_loglog.png", dpi=300)
plt.show()
