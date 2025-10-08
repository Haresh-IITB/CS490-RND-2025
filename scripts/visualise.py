import pandas as pd
import networkx as nx
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
import os

def visualize_simulation_data(
    node_file="results/graph/nodes.csv", 
    edge_file="results/graph/edges.csv", 
    infected_file="results/infected.csv", 
    vaccinated_file="results/vaccinated.csv",
    results_file="results/results.csv"
):

    # --- Data Loading and Validation ---
    required_files = [node_file, edge_file, infected_file, vaccinated_file, results_file]
    for f in required_files:
        if not os.path.exists(f):
            print(f"Error: Required file '{f}' not found.")
            print("Please make sure you have run the C++ simulation program first to generate the necessary CSV files.")
            return

    nodes_df = pd.read_csv(node_file)
    edges_df = pd.read_csv(edge_file)
    infected_df = pd.read_csv(infected_file)
    vaccinated_df = pd.read_csv(vaccinated_file)

    infected_nodes = set(infected_df['node_id'])
    vaccinated_nodes = set(vaccinated_df['node_id'])
    
    # ---  Build the Graph ---
    G = nx.Graph()
    positions = {}
    node_colors = {}
    node_sizes = {}
    
    for index, row in nodes_df.iterrows():
        node_id = index
        G.add_node(node_id)
        positions[node_id] = (row['x'], row['y'])
        
        if node_id in infected_nodes:
            node_colors[node_id] = '#E63946' 
            node_sizes[node_id] = 250
        elif node_id in vaccinated_nodes:
            node_colors[node_id] = '#52B69A' 
            node_sizes[node_id] = 250
        elif row['type'] == 'city':
            node_colors[node_id] = '#FCA311' 
            node_sizes[node_id] = 150
        else:
            node_colors[node_id] = '#A8DADC'
            node_sizes[node_id] = 50

    # Excluding the header 
    for _, row in edges_df.iterrows():
        G.add_edge(row['source'], row['target'])

    print(f"Graph loaded with {G.number_of_nodes()} nodes and {G.number_of_edges()} edges.")

    # ---  The Network Graph ---
    plt.style.use('seaborn-v0_8-whitegrid')
    plt.figure(figsize=(16, 16))
    
    color_list = [node_colors[node] for node in G.nodes()]
    size_list = [node_sizes[node] for node in G.nodes()]

    nx.draw_networkx_edges(G, pos=positions, alpha=0.4, edge_color='gray')
    nx.draw_networkx_nodes(G, pos=positions, node_color=color_list, node_size=size_list, edgecolors='black', linewidths=0.5)
    
    plt.title("Simulation Network State", fontsize=24, fontweight='bold', pad=20)
    plt.xlabel("X Coordinate", fontsize=14)
    plt.ylabel("Y Coordinate", fontsize=14)
    plt.axis('on')
    plt.tick_params(left=True, bottom=True, labelleft=True, labelbottom=True)
    
    legend_elements = [
        Line2D([0], [0], marker='o', color='w', label='Infected', markerfacecolor='#E63946', markersize=14, markeredgecolor='black'),
        Line2D([0], [0], marker='o', color='w', label='Vaccinated', markerfacecolor='#52B69A', markersize=14, markeredgecolor='black'),
        Line2D([0], [0], marker='o', color='w', label='City', markerfacecolor='#FCA311', markersize=12, markeredgecolor='black'),
        Line2D([0], [0], marker='o', color='w', label='Healthy Node', markerfacecolor='#A8DADC', markersize=10, markeredgecolor='gray')
    ]
    plt.legend(handles=legend_elements, loc='best', title="Node Status", fontsize=12, title_fontsize=14)
    plt.tight_layout()
    plt.show()



if __name__ == "__main__":
    visualize_simulation_data()

