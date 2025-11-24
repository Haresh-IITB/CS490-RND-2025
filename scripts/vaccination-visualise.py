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

def plot_efficacy_split(df):
    """
    Plot 1: Efficacy - Split by Model (IC vs LT)
    Visual Change: Used a Point Plot instead of Bar Plot for cleaner comparison.
    """
    # Calculate % Saved
    df['Percent_Saved'] = (df['Nodes_Saved'] / df['Nodes_N']) * 100
    
    # Define models to iterate over
    models = ['IC', 'LT']
    
    for model in models:
        subset = df[df['Model'] == model]
        
        plt.figure(figsize=(10, 6))
        sns.set_theme(style="whitegrid")
        
        # SUBSTITUTION: Using pointplot instead of barplot
        # dodge=True separates the lines slightly so they don't overlap
        sns.pointplot(
            data=subset, x="Budget_Percent", y="Percent_Saved", hue="Algorithm",
            dodge=0.4, capsize=0.1, palette="viridis", 
            markers=["o", "s", "D", "^"], linestyles=["-", "--", "-.", ":"]
        )
        
        plt.title(f"Vaccination Efficacy ({model} Model): % Nodes Saved vs Budget", fontsize=16)
        plt.xlabel("Vaccination Budget (%)", fontsize=12)
        plt.ylabel("Nodes Saved (%)", fontsize=12)
        plt.legend(title="Algorithm", bbox_to_anchor=(1.05, 1), loc='upper left')
        
        save_path = os.path.join(OUTPUT_DIR, f"1_efficacy_{model}.png")
        plt.savefig(save_path, bbox_inches='tight', dpi=150)
        plt.close()
        print(f"Saved {save_path}")

def plot_time_complexity_split(df):
    """
    Plot 2: Time Complexity - Split by Model (IC vs LT)
    Budget fixed at 10% for clarity.
    """
    # Filter for one budget to clean up the plot
    df_budget = df[df['Budget_Percent'] == 10]
    
    models = ['IC', 'LT']
    
    for model in models:
        subset = df_budget[df_budget['Model'] == model]
        
        plt.figure(figsize=(10, 6))
        sns.set_theme(style="whitegrid")
        
        sns.lineplot(
            data=subset, x="Nodes_N", y="Time_ms", hue="Algorithm",
            markers=True, style="Algorithm", dashes=False, linewidth=2.5, palette="deep"
        )
        
        plt.title(f"Time Complexity Analysis ({model} Model) [Budget=10%]", fontsize=16)
        plt.xlabel("Number of Nodes (N)", fontsize=12)
        plt.ylabel("Execution Time (ms)", fontsize=12)
        plt.yscale('log') 
        plt.grid(True, which="both", ls="--", alpha=0.5)
        
        save_path = os.path.join(OUTPUT_DIR, f"2_time_complexity_{model}.png")
        plt.savefig(save_path, bbox_inches='tight', dpi=150)
        plt.close()
        print(f"Saved {save_path}")

def plot_robustness_split(df):
    """
    Plot 3: Robustness (Boxplot) - Split by Model (IC vs LT)
    """
    # Normalize saved nodes
    df['Ratio_Saved'] = df['Nodes_Saved'] / df['Nodes_N']
    
    models = ['IC', 'LT']
    
    for model in models:
        subset = df[df['Model'] == model]
        
        plt.figure(figsize=(10, 6))
        sns.set_theme(style="whitegrid")
        
        # We keep hue="Algorithm" just for the color palette, but turn off the legend 
        # since the x-axis already labels them.
        sns.boxplot(
            data=subset, x="Algorithm", y="Ratio_Saved", 
            hue="Algorithm", palette="Set2", dodge=False
        )
        
        plt.title(f"Algorithm Robustness ({model} Model)", fontsize=16)
        plt.ylabel("Fraction of Network Saved (0.0 - 1.0)", fontsize=12)
        plt.xlabel("Algorithm", fontsize=12)
        
        # Move legend out or remove it if redundant (removing here as X-axis labels are sufficient)
        plt.legend([],[], frameon=False)
        
        save_path = os.path.join(OUTPUT_DIR, f"3_algo_robustness_{model}.png")
        plt.savefig(save_path, bbox_inches='tight', dpi=150)
        plt.close()
        print(f"Saved {save_path}")

if __name__ == "__main__":
    ensure_dir(OUTPUT_DIR)
    
    if not os.path.exists(INPUT_FILE):
        print(f"Error: {INPUT_FILE} not found. Run the C++ benchmark first.")
    else:
        df = pd.read_csv(INPUT_FILE)
        
        print("Generating Split Efficacy Plots (Point Plot)...")
        plot_efficacy_split(df)
        
        print("Generating Split Time Complexity Plots...")
        plot_time_complexity_split(df)
        
        print("Generating Split Robustness Plots...")
        plot_robustness_split(df)