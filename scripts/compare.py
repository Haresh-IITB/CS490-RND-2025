import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import numpy as np

def round_to_nearest_power_of_2(n):
    if n <= 0:
        return 0
    power = round(np.log2(n))
    return int(2**power)

def create_visualizations(csv_file='results/experiment_results.csv'):

    # Load Data and Prepare Environment 
    try:
        df = pd.read_csv(csv_file)
    except FileNotFoundError:
        print(f"Error: The file '{csv_file}' was not found.")
        print("Please make sure the CSV file is in the same directory as this script.")
        return

    df['NodeMagnitude'] = df['NumNodes'].apply(round_to_nearest_power_of_2)

    # Create a directory to save the plots
    output_dir = 'results/visualizations'
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    print(f"Data from '{csv_file}' loaded successfully.")
    print("Grouping node counts by the nearest power of 2 for analysis.")
    print("Generating plots and saving them to the 'visualizations' directory...")

    # Log of Simulation Time vs. Number of Nodes (by Magnitude) 
    plt.style.use('seaborn-v0_8-whitegrid')
    fig1, ax1 = plt.subplots(figsize=(12, 8))
    time_vs_nodes = df.groupby(['Algorithm', 'NodeMagnitude'])['TimeTaken_ms'].mean().reset_index()

    for algorithm in time_vs_nodes['Algorithm'].unique():
        subset = time_vs_nodes[time_vs_nodes['Algorithm'] == algorithm]
        ax1.plot(subset['NodeMagnitude'], subset['TimeTaken_ms'], marker='o', linestyle='-', label=algorithm)

    ax1.set_yscale('log')
    ax1.set_title('Algorithm Execution Time vs. Graph Size (Grouped by Magnitude)', fontsize=16)
    ax1.set_xlabel('Number of Nodes (Grouped by nearest power of 2)', fontsize=12)
    ax1.set_ylabel('Average Time Taken (ms, log scale)', fontsize=12)
    ax1.legend(title='Algorithm')
    ax1.grid(True, which="both", ls="--")
    
    plot1_path = os.path.join(output_dir, 'time_vs_nodes_by_magnitude.png')
    fig1.savefig(plot1_path)
    plt.close(fig1)
    print(f"  - Saved: {plot1_path}")

    # Plot 2: Nodes Saved vs. K_Budget (by Magnitude)
    # Create a separate plot for each graph size magnitude.
    for node_magnitude in sorted(df['NodeMagnitude'].unique()):
        plt.style.use('seaborn-v0_8-whitegrid')
        fig2, ax2 = plt.subplots(figsize=(12, 8))
        
        subset_df = df[df['NodeMagnitude'] == node_magnitude]
        
        sns.barplot(
            data=subset_df,
            x='K_Percent',
            y='NodesSaved',
            hue='Algorithm',
            ax=ax2,
            palette='viridis'
        )
        
        ax2.set_title(f'Nodes Saved vs. Vaccination Budget (for ~{node_magnitude} Nodes)', fontsize=16)
        ax2.set_xlabel('Vaccination Budget (K %)', fontsize=12)
        ax2.set_ylabel('Average Nodes Saved', fontsize=12)
        ax2.legend(title='Algorithm')
        
        plot2_path = os.path.join(output_dir, f'nodes_saved_vs_k_budget_{node_magnitude}_nodes.png')
        fig2.savefig(plot2_path)
        plt.close(fig2)
        print(f"  - Saved: {plot2_path}")


    #  Line Plot for Each Algorithm 
    algorithms = df['Algorithm'].unique()
    for algorithm in algorithms:
        plt.style.use('seaborn-v0_8-whitegrid')
        fig3, ax3 = plt.subplots(figsize=(11, 8))
        algo_df = df[df['Algorithm'] == algorithm]
        
        sns.lineplot(
            data=algo_df,
            x='NodeMagnitude',
            y='NodesSaved',
            hue='K_Percent',
            style='K_Percent', 
            palette=['#6A0DAD', '#4169E1', '#006400'],
            marker='o',
            markersize=8,
            ax=ax3,
            legend='full',
            errorbar=None
        )
        
        ax3.set_title(f'Performance of {algorithm} Algorithm', fontsize=18, pad=20)
        ax3.set_xlabel('Number of Nodes (Grouped by nearest power of 2)', fontsize=12)
        ax3.set_ylabel('Number of Nodes Saved', fontsize=12)
        ax3.grid(True, which="both", ls="--")
        
        ax3.set_xticks(sorted(df['NodeMagnitude'].unique()))
        
        ax3.legend(title='K Budget (%)', fontsize='11')
        
        plot3_path = os.path.join(output_dir, f'performance_plot_{algorithm.replace(" ", "_")}.png')
        fig3.savefig(plot3_path)
        plt.close(fig3)
        print(f"  - Saved: {plot3_path}")
        
    print("\nAll visualizations have been created successfully.")


if __name__ == '__main__':
    create_visualizations('results/experiment_results.csv')

