import pandas as pd
import matplotlib.pyplot as plt

nodes = pd.read_csv("results/debug_nodes.csv")
edges = pd.read_csv("results/debug_edges.csv")

plt.figure(figsize=(7, 7))

# ---- Plot edges ----
for _, e in edges.iterrows():
    u = nodes.loc[nodes.id == e.u].iloc[0]
    v = nodes.loc[nodes.id == e.v].iloc[0]
    plt.plot([u.x, v.x], [u.y, v.y],
             color="gray", linewidth=0.7, alpha=0.6)

# ---- Plot nodes ----
for _, n in nodes.iterrows():
    if n.vaccinated == 1:
        plt.scatter(n.x, n.y, c="green", s=120, marker="X", label="Vaccinated")
    elif n.infected == 1:
        plt.scatter(n.x, n.y, c="red", s=120, marker="*", label="Infected")
    elif n.tier == 1:
        plt.scatter(n.x, n.y, c="blue", s=60, label="City")
    else:
        plt.scatter(n.x, n.y, c="orange", s=40, label="Village")

    # ---- ADD NODE ID LABEL ----
    plt.text(
        n.x + 0.002,          # small x offset
        n.y + 0.002,          # small y offset
        str(n.id),
        fontsize=9,
        color="black",
        ha="left",
        va="bottom"
    )

# Remove duplicate legend entries
handles, labels = plt.gca().get_legend_handles_labels()
by_label = dict(zip(labels, handles))
plt.legend(by_label.values(), by_label.keys())

plt.title("Greedy + IC Debug Graph (2-Tier)")
plt.xlabel("x")
plt.ylabel("y")
plt.axis("equal")
plt.grid(True)
plt.tight_layout()
plt.show()
