import pandas as pd
import matplotlib.pyplot as plt

# -------------------------------------------------
# Load CSV
# -------------------------------------------------
csv_path = "results/benchmark_stepsize_256.csv"
df = pd.read_csv(csv_path)

step_sizes = df["stepsize"]

# Algorithms: (column_name, label)
algorithms = [
    ("baseline", "Baseline"),
    ("greedy", "Greedy"),
    ("local_search", "Local Search"),
    ("hill_climbing", "Hill Climbing"),
    ("lp_tkr", "LP-TKR"),
    ("lp_irp", "LP-IRP"),
]

# -------------------------------------------------
# Plot
# -------------------------------------------------
plt.figure(figsize=(8, 6))

for col, label in algorithms:
    plt.plot(
        step_sizes,
        df[col],
        marker="o",
        linewidth=2,
        markersize=6,
        label=label
    )

# -------------------------------------------------
# Formatting
# -------------------------------------------------
plt.xlabel("Step Size (movement interval)")
plt.ylabel("Nodes Saved (%)")
plt.title("Effect of Movement Frequency on Vaccination Performance")

plt.grid(True, linestyle="--", alpha=0.6)
plt.legend()

plt.tight_layout()
plt.savefig("results/benchmark_stepsize_saved.png", dpi=300)
plt.show()
