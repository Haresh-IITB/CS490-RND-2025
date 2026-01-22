import pandas as pd
import matplotlib.pyplot as plt

def plot_lp_tkr_results():
    percent_saved = input(
        "Enter the fraction of vaccination (e.g. 0.10 for 10%): "
    ).strip()

    # Load CSVs
    nodes_file = f'./results/dynamic_lp_nodes_saved_{percent_saved}.csv'
    time_file  = f'./results/dynamic_lp_time_taken_{percent_saved}.csv'

    df_nodes = pd.read_csv(nodes_file)
    df_time  = pd.read_csv(time_file)

    strategies = [
        "Uniform",
        "FrontLoaded",
        "BackLoaded",
        "StaticOneShot",
        "WithoutVaccine"
    ]

    # -------------------------------
    # Plot 1: Nodes Saved vs NodeSize
    # -------------------------------
    plt.figure()
    for strat in strategies:
        plt.plot(
            df_nodes["NodeSize"],
            df_nodes[strat],
            marker='o',
            label=strat
        )

    plt.xlabel("Number of Nodes (N)")
    plt.ylabel("Nodes Saved")
    plt.title(f"LP with TKR: Nodes Saved vs Graph Size\nVaccination Fraction = {percent_saved}")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    plt.savefig(
        f'./results/lp_tkr_nodes_saved_{percent_saved}.png',
        dpi=300
    )
    plt.show()

    # -------------------------------
    # Plot 2: Runtime vs NodeSize
    # -------------------------------
    plt.figure()
    for strat in strategies:
        plt.plot(
            df_time["NodeSize"],
            df_time[strat],
            marker='o',
            label=strat
        )

    plt.xlabel("Number of Nodes (N)")
    plt.ylabel("Runtime (seconds)")
    plt.title(f"LP with TKR: Runtime vs Graph Size\nVaccination Fraction = {percent_saved}")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    plt.savefig(
        f'./results/lp_tkr_runtime_{percent_saved}.png',
        dpi=300
    )
    plt.show()


if __name__ == "__main__":
    plot_lp_tkr_results()
