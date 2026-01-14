import pandas as pd
import matplotlib.pyplot as plt

# -----------------------------
# Load data
# -----------------------------
csv_path = "results/benchmark_results.csv"
df = pd.read_csv(csv_path)

nodes = df["nodes"]

# Algorithms (column, label)
algos = [
    ("baseline", "Baseline"),
    ("greedy", "Greedy"),
    ("local_search", "Local Search"),
    ("hill_climbing", "Hill Climbing"),
    ("lp_tkr", "LP-TKR"),
    ("lp_irp", "LP-IRP"),
]

# -----------------------------
# Convert to percentage saved
# -----------------------------
plt.figure(figsize=(8, 6))

for col, label in algos:
    percent_saved = (df[col] / nodes) * 100.0
    plt.plot(
        nodes,
        percent_saved,
        marker="o",
        linewidth=2,
        label=label
    )

# -----------------------------
# Plot formatting
# -----------------------------
plt.xlabel("Number of Nodes")
plt.ylabel("Nodes Saved (%)")
plt.title("Percentage of Nodes Saved vs Graph Size")

plt.grid(True, linestyle="--", alpha=0.6)
plt.legend()

plt.tight_layout()
plt.savefig("results/benchmark_percent_saved.png", dpi=300)
plt.show()
