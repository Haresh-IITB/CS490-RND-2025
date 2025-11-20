import matplotlib.pyplot as plt
import pandas as pd
import networkx as nx
import os
import re
import matplotlib.lines as mlines

# Paths
INPUT_DIR = "results/graph-results/"
OUTPUT_DIR = "results/graph-visualisation/"

def ensure_dir(d):
    if not os.path.exists(d):
        os.makedirs(d)

def load_graph(node_file, edge_file):
    try:
        nodes_df = pd.read_csv(node_file)
        edges_df = pd.read_csv(edge_file)
    except FileNotFoundError:
        return None, None, None
    
    G = nx.Graph()
    pos = {}
    # Tier colors: Village(0)=Blue, T3(1)=Green, T2(2)=Orange, T1(3)=Red
    color_map = {0: '#3498db', 1: '#2ecc71', 2: '#f39c12', 3: '#e74c3c'}
    
    for _, row in nodes_df.iterrows():
        nid = int(row['id'])
        G.add_node(nid, tier=row['tier'])
        pos[nid] = (row['x'], row['y'])
    
    for _, row in edges_df.iterrows():
        G.add_edge(int(row['u']), int(row['v']))
        
    node_colors = [color_map.get(G.nodes[n]['tier'], 'gray') for n in G.nodes]
    return G, pos, node_colors

def plot_param_variations():
    # Regex to find files matching alpha_X.XX_beta_X.XX format
    files = [f for f in os.listdir(INPUT_DIR) if f.startswith("alpha_") and f.endswith("_nodes.csv")]
    files.sort() # Ensure consistent order
    
    if not files:
        print("No parameter variation files found.")
        return

    # 2x2 Subplot
    fig, axes = plt.subplots(2, 2, figsize=(12, 12))
    fig.suptitle('Impact of Alpha and Beta on Graph Topology', fontsize=16)
    axes = axes.flatten()

    for i, node_filename in enumerate(files):
        if i >= 4: break
        
        base_name = node_filename.replace("_nodes.csv", "")
        edge_filename = base_name + "_edges.csv"
        
        # Extract params for title
        # Expected format: alpha_0.05_beta_0.10
        title = base_name.replace("alpha_", "α=").replace("_beta_", ", β=")

        G, pos, colors = load_graph(os.path.join(INPUT_DIR, node_filename), 
                                  os.path.join(INPUT_DIR, edge_filename))
        
        if G is None: continue
        
        ax = axes[i]
        nx.draw(G, pos, ax=ax, node_size=15, node_color=colors, edge_color='gray', alpha=0.5, width=0.3)
        ax.set_title(title, fontsize=12)
        ax.set_aspect('equal')
        ax.set_xticks([])
        ax.set_yticks([])

    plt.tight_layout()
    plt.subplots_adjust(top=0.95)
    save_path = os.path.join(OUTPUT_DIR, "1_param_variations_2x2.png")
    plt.savefig(save_path, dpi=150)
    plt.close()
    print(f"Saved {save_path}")

def plot_movement():
    start_nodes = os.path.join(INPUT_DIR, "move_start_nodes.csv")
    end_nodes = os.path.join(INPUT_DIR, "move_end_nodes.csv")
    
    if not os.path.exists(start_nodes): 
        print("Movement files not found.")
        return

    df_start = pd.read_csv(start_nodes).sort_values('id')
    df_end = pd.read_csv(end_nodes).sort_values('id')
    
    # Define Color Pairs: {Tier: (FinalColor, InitialColor)}
    # InitialColor is a lighter/pastel version of FinalColor
    tier_styles = {
        3: {'final': '#e74c3c', 'initial': '#f5b7b1', 'name': 'Tier 1 (City)'}, # Red / Pink
        2: {'final': '#f39c12', 'initial': '#fdebd0', 'name': 'Tier 2'},        # Orange / Pale Orange
        1: {'final': '#2ecc71', 'initial': '#abebc6', 'name': 'Tier 3'},        # Green / Pale Green
        0: {'final': '#3498db', 'initial': '#d6eaf8', 'name': 'Village'}        # Blue / Pale Blue
    }

    # Generate color lists based on the FINAL tier of the node
    # This visualizes "Where did the nodes that ended up in Tier X come from?"
    final_colors = [tier_styles[t]['final'] for t in df_end['tier']]
    initial_colors = [tier_styles[t]['initial'] for t in df_end['tier']]

    plt.figure(figsize=(12, 12))
    ax = plt.gca()
    
    # 1. Plot Movement Vectors (Lines)
    # We color the lines faintly matching the final tier to show flow direction
    move_threshold = 0.005
    for i in range(len(df_start)):
        x1, y1 = df_start.iloc[i]['x'], df_start.iloc[i]['y']
        x2, y2 = df_end.iloc[i]['x'], df_end.iloc[i]['y']
        dist = ((x2-x1)**2 + (y2-y1)**2)**0.5
        
        if dist > move_threshold:
            # Use the initial color for the line so it looks like a trail
            line_color = initial_colors[i]
            ax.plot([x1, x2], [y1, y2], color=line_color, alpha=0.6, linewidth=0.8, zorder=1)

    # 2. Plot INITIAL Positions (Lighter Color)
    ax.scatter(df_start['x'], df_start['y'], c=initial_colors, 
               s=30, label='Initial Position', alpha=0.8, edgecolor='none', zorder=2)

    # 3. Plot FINAL Positions (Vibrant Color)
    ax.scatter(df_end['x'], df_end['y'], c=final_colors, 
               s=40, label='Final Position', alpha=1.0, edgecolor='white', linewidth=0.5, zorder=3)

    # Styling
    ax.set_title("Superimposed Node Movement (Initial → Final)", fontsize=16)
    ax.set_aspect('equal')
    ax.grid(True, linestyle=':', alpha=0.3)

    # Custom Legend Construction
    legend_handles = []
    
    # Header for Markers
    legend_handles.append(mlines.Line2D([], [], color='none', label=r'$\bf{Movement\ (Start \to End)}$'))
    
    for tier in sorted(tier_styles.keys(), reverse=True):
        style = tier_styles[tier]
        # Create a paired marker handle: "Initial Color o -> Final Color O"
        # We can't easily do two markers in one legend entry, so we use text description
        # or we just show the colors.
        
        # Let's add the pair explicitly
        handle = mlines.Line2D([], [], color=style['final'], marker='o', 
                              markerfacecolor=style['final'], 
                              markeredgecolor=style['initial'], # Ring of initial color
                              markeredgewidth=2,
                              markersize=8, 
                              label=style['name'])
        legend_handles.append(handle)

    # Explanatory legend items
    legend_handles.append(mlines.Line2D([], [], color='none', label=' '))
    legend_handles.append(mlines.Line2D([0], [0], marker='o', color='w', markerfacecolor='gray', alpha=0.4, label='Start Pos (Light)'))
    legend_handles.append(mlines.Line2D([0], [0], marker='o', color='w', markerfacecolor='black', label='End Pos (Dark)'))
    
    ax.legend(handles=legend_handles, loc='upper right', frameon=True, framealpha=0.9)

    plt.tight_layout()
    save_path = os.path.join(OUTPUT_DIR, "2_movement_dynamics_superimposed.png")
    plt.savefig(save_path, dpi=150)
    plt.close()
    print(f"Saved {save_path}")

def plot_time_analysis():
    time_file = os.path.join(INPUT_DIR, "time_analysis.csv")
    if not os.path.exists(time_file): 
        print("Time analysis file not found.")
        return
    
    df = pd.read_csv(time_file)
    
    plt.figure(figsize=(10, 6))
    plt.plot(df['N'], df['Time_ms'], 'o-', linewidth=2, color='#8e44ad')
    plt.title("Graph Generation Time Complexity", fontsize=14)
    plt.xlabel("Number of Nodes (N)", fontsize=12)
    plt.ylabel("Time (ms)", fontsize=12)
    plt.grid(True, which="both", ls="-", alpha=0.2)
    
    # Add value annotations
    for x, y in zip(df['N'], df['Time_ms']):
        label = f"{y:.0f}ms"
        plt.annotate(label, (x, y), textcoords="offset points", xytext=(0,10), ha='center')

    save_path = os.path.join(OUTPUT_DIR, "3_time_analysis.png")
    plt.savefig(save_path, dpi=150)
    plt.close()
    print(f"Saved {save_path}")

if __name__ == "__main__":
    ensure_dir(OUTPUT_DIR)
    plot_param_variations()
    plot_movement()
    plot_time_analysis()