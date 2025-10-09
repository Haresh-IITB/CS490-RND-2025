import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import networkx as nx
from matplotlib.lines import Line2D
import os

def visualize_network_state(
    node_file="results/graph/nodes.csv", 
    edge_file="results/graph/edges.csv", 
    infected_file="results/infected.csv", 
    vaccinated_file="results/vaccinated.csv"
):
    print("--- Generating Network State Visualization ---")
    
    # Data Loading and Validation
    required_files = [node_file, edge_file, infected_file, vaccinated_file]
    for f in required_files:
        if not os.path.exists(f):
            print(f"Error: Required file '{f}' not found.")
            print("Please run the C++ simulation first to generate visualization data.")
            return

    nodes_df = pd.read_csv(node_file)
    edges_df = pd.read_csv(edge_file)
    infected_df = pd.read_csv(infected_file)
    vaccinated_df = pd.read_csv(vaccinated_file)

    infected_nodes = set(infected_df['node_id'])
    vaccinated_nodes = set(vaccinated_df['node_id'])

    G = nx.Graph()
    positions = {}
    node_colors = {}
    node_sizes = {}
    
    for node_id, row in nodes_df.iterrows():
        G.add_node(node_id)
        positions[node_id] = (row['x'], row['y'])
        
        if node_id in infected_nodes:
            node_colors[node_id] = '#E63946' # Red
            node_sizes[node_id] = 200
        elif node_id in vaccinated_nodes:
            node_colors[node_id] = '#52B69A' # Green
            node_sizes[node_id] = 200
        else:
            node_colors[node_id] = '#A8DADC' # Light Blue
            node_sizes[node_id] = 40

    for _, row in edges_df.iterrows():
        G.add_edge(row['source'], row['target'])

    print(f"Graph loaded with {G.number_of_nodes()} nodes and {G.number_of_edges()} edges.")

    # Plotting the Network Graph
    plt.style.use('seaborn-v0_8-whitegrid')
    fig, ax = plt.subplots(figsize=(14, 14))
    
    color_list = [node_colors.get(node, '#A8DADC') for node in G.nodes()]
    size_list = [node_sizes.get(node, 40) for node in G.nodes()]

    nx.draw_networkx_edges(G, pos=positions, ax=ax, alpha=0.3, edge_color='gray')
    nx.draw_networkx_nodes(G, pos=positions, ax=ax, node_color=color_list, node_size=size_list, edgecolors='black', linewidths=0.5)
    
    ax.set_title("Final Simulation Network State", fontsize=22, fontweight='bold', pad=20)
    ax.set_xlabel("X Coordinate", fontsize=12)
    ax.set_ylabel("Y Coordinate", fontsize=12)
    ax.tick_params(left=True, bottom=True, labelleft=True, labelbottom=True)
    
    legend_elements = [
        Line2D([0], [0], marker='o', color='w', label='Infected', markerfacecolor='#E63946', markersize=12, markeredgecolor='black'),
        Line2D([0], [0], marker='o', color='w', label='Vaccinated', markerfacecolor='#52B69A', markersize=12, markeredgecolor='black'),
        Line2D([0], [0], marker='o', color='w', label='Healthy', markerfacecolor='#A8DADC', markersize=10, markeredgecolor='gray')
    ]
    ax.legend(handles=legend_elements, loc='best', title="Node Status", fontsize=11, title_fontsize=13)
    plt.tight_layout()
    plt.show()


def plot_performance_graphs(results_file="results/experiment_results.csv"):

    print("\n--- Generating Performance Plots from experiment_results.csv ---")
    
    if not os.path.exists(results_file):
        print(f"Error: Results file '{results_file}' not found.")
        return

    df = pd.read_csv(results_file)
    sns.set_theme(style="whitegrid")
    
    df_time = df[(df['K_Percent'] == 10) & (df['Infected_Percent'] == 10)]
    
    if not df_time.empty:
        pivot_time = df_time.pivot_table(index='NumNodes', columns='Algorithm', values='TimeTaken_ms')
        
        plt.figure(figsize=(10, 6))
        ax = pivot_time.plot(logy=True, logx=True, marker='o', linestyle='-')
        ax.set_title('Log-Log: Time Taken vs. Network Size', fontsize=16, fontweight='bold')
        ax.set_xlabel('Total Number of Nodes (log scale)', fontsize=12)
        ax.set_ylabel('Time Taken in ms (log scale)', fontsize=12)
        ax.legend(title='Algorithm')
        ax.grid(True, which="both", ls="--")
        plt.tight_layout()
        plt.show()

    largest_node_size = df['NumNodes'].max()
    df_budget = df[(df['NumNodes'] == largest_node_size) & (df['Infected_Percent'] == 10)]

    if not df_budget.empty:
        pivot_budget = df_budget.pivot_table(index='K_Percent', columns='Algorithm', values='NodesSaved')
        
        plt.figure(figsize=(10, 6))
        ax = pivot_budget.plot(marker='o', linestyle='-')
        ax.set_title(f'Nodes Saved vs. Vaccination Budget\n(For {largest_node_size}-Node Graph)', fontsize=16, fontweight='bold')
        ax.set_xlabel('Vaccination Budget (%)', fontsize=12)
        ax.set_ylabel('Number of Nodes Saved', fontsize=12)
        ax.legend(title='Algorithm')
        ax.grid(True, which="both", ls="--")
        plt.tight_layout()
        plt.show()

if __name__ == "__main__":
    os.makedirs("results/graph", exist_ok=True)
    visualize_network_state()
    plot_performance_graphs()