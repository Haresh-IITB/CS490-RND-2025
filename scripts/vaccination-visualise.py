import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
import os

# Paths
INPUT_FILE = "results/task2_analysis/benchmark_results.csv"
OUTPUT_DIR = "results/task2_visualisation/"

def ensure_dir(d):
    if not os.path.exists(d):
        os.makedirs(d)

def plot_efficacy(df):
    """
    Plot 1: Nodes Saved vs Budget (K%)
    Facetted by Model (IC vs LT)
    """
    # Calculate % Saved for fair comparison across N
    df['Percent_Saved'] = (df['Nodes_Saved'] / df['Nodes_N']) * 100
    
    # We'll take the average across all N for the efficacy plot to show general trend
    plt.figure(figsize=(14, 6))
    sns.set_theme(style="whitegrid")
    
    g = sns.catplot(
        data=df, x="Budget_Percent", y="Percent_Saved", hue="Algorithm", col="Model",
        kind="bar", height=5, aspect=1.2, palette="viridis", alpha=0.9
    )
    
    g.fig.suptitle('Vaccination Efficacy: % Nodes Saved vs Budget', y=1.05, fontsize=16)
    g.set_axis_labels("Vaccination Budget (%)", "Nodes Saved (%)")
    
    save_path = os.path.join(OUTPUT_DIR, "1_efficacy_vs_budget.png")
    plt.savefig(save_path, bbox_inches='tight', dpi=150)
    plt.close()
    print(f"Saved {save_path}")

def plot_time_complexity(df):
    """
    Plot 2: Execution Time vs Graph Size (N)
    Facetted by Algorithm (to see scaling) or Combined
    """
    plt.figure(figsize=(12, 6))
    
    # Filter for one budget (e.g., 10%) to clean up the plot
    subset = df[df['Budget_Percent'] == 10]
    
    # Line plot with markers
    sns.lineplot(
        data=subset, x="Nodes_N", y="Time_ms", hue="Algorithm", style="Model",
        markers=True, dashes=False, linewidth=2.5, palette="deep"
    )
    
    plt.title("Time Complexity Analysis (Budget=10%)", fontsize=16)
    plt.xlabel("Number of Nodes (N)", fontsize=12)
    plt.ylabel("Execution Time (ms)", fontsize=12)
    plt.yscale('log') # Log scale because Greedy explodes
    plt.grid(True, which="both", ls="--", alpha=0.5)
    
    save_path = os.path.join(OUTPUT_DIR, "2_time_complexity_log.png")
    plt.savefig(save_path, bbox_inches='tight', dpi=150)
    plt.close()
    print(f"Saved {save_path}")

def plot_model_comparison(df):
    """
    Plot 3: Algorithm ranking summary (Boxplot of saved nodes)
    """
    plt.figure(figsize=(10, 6))
    
    # Normalize saved nodes by N to get comparable ratios
    df['Ratio_Saved'] = df['Nodes_Saved'] / df['Nodes_N']
    
    sns.boxplot(data=df, x="Algorithm", y="Ratio_Saved", hue="Model", palette="Set2")
    
    plt.title("Overall Algorithm Robustness across all N and K", fontsize=16)
    plt.ylabel("Fraction of Network Saved (0.0 - 1.0)")
    
    save_path = os.path.join(OUTPUT_DIR, "3_algo_robustness.png")
    plt.savefig(save_path, bbox_inches='tight', dpi=150)
    plt.close()
    print(f"Saved {save_path}")

if __name__ == "__main__":
    ensure_dir(OUTPUT_DIR)
    
    if not os.path.exists(INPUT_FILE):
        print(f"Error: {INPUT_FILE} not found. Run the C++ benchmark first.")
    else:
        df = pd.read_csv(INPUT_FILE)
        plot_efficacy(df)
        plot_time_complexity(df)
        plot_model_comparison(df)