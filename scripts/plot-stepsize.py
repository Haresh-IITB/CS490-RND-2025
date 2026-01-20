import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from glob import glob
import os

def generate_benchmark_plot():
    # 1. Load Data from ./results directory
    # The glob pattern now looks inside the 'results' folder
    search_path = os.path.join(".", "results", "*.csv")
    files = glob(search_path)
    
    data_nodes = []
    data_time = []

    print(f"Reading files from {search_path}...")
    
    for f in files:
        try:
            # Extract filename from the full path
            filename = os.path.basename(f)
            
            # Extract size (N) from filename 
            # Logic: split by '_' and take the last part (removing .csv extension)
            # Example: 'benchmark_stepsize_2048.csv' -> '2048'
            parts = filename.replace('.csv', '').split('_')
            
            if parts[-1].isdigit():
                size = int(parts[-1])
            else:
                # If the filename format is unexpected, skip it
                print(f"Skipping {filename}: Number of nodes not found in filename.")
                continue

            df = pd.read_csv(f)
            
            # Filter for stepsize = 3
            if 'stepsize' in df.columns:
                row = df[df['stepsize'] == 3]
            else:
                continue

            if not row.empty:
                r = row.iloc[0].to_dict()
                r['num_nodes'] = size
                
                # Distinguish between time and node-saved files based on filename
                if 'time' in filename:
                    data_time.append(r)
                else:
                    data_nodes.append(r)
        except Exception as e:
            print(f"Error reading {f}: {e}")

    # Create DataFrames
    df_nodes = pd.DataFrame(data_nodes).sort_values('num_nodes')
    df_time = pd.DataFrame(data_time).sort_values('num_nodes')

    # 2. Extrapolation Logic for Time
    # Algorithms that are missing for >= 1024 nodes
    algos_to_extrap = ['greedy', 'local_search', 'hill_climbing']
    extrapolated_points = {algo: [] for algo in algos_to_extrap}

    def fit_log_log(x, y, x_pred):
        """Fit a linear model in log-log space: log(y) = m * log(x) + c"""
        log_x = np.log(x)
        log_y = np.log(y)
        coeffs = np.polyfit(log_x, log_y, 1)
        log_y_pred = coeffs[0] * np.log(x_pred) + coeffs[1]
        return np.exp(log_y_pred)

    for algo in algos_to_extrap:
        if algo in df_time.columns:
            # Get existing valid data for fitting
            valid_data = df_time.dropna(subset=[algo])
            
            # We need at least 2 points to extrapolate
            if len(valid_data) >= 2:
                X = valid_data['num_nodes'].values
                Y = valid_data[algo].values
                
                # Identify missing rows (NaN values) for this algorithm
                missing_mask = df_time[algo].isna()
                missing_nodes = df_time.loc[missing_mask, 'num_nodes'].values
                
                if len(missing_nodes) > 0:
                    # Predict
                    preds = fit_log_log(X, Y, missing_nodes)
                    
                    # Fill DataFrame
                    df_time.loc[missing_mask, algo] = preds
                    
                    # Store for plotting (to mark with hollow dots)
                    for n, p in zip(missing_nodes, preds):
                        extrapolated_points[algo].append((n, p))
                    
                    print(f"Extrapolated {algo} for nodes {missing_nodes}")

    # 3. Plotting
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 10), sharex=True)

    # Define styles for consistency
    # keys match the csv column names
    styles = {
        'greedy': {'color': '#d62728', 'marker': 'o', 'label': 'Greedy'},
        'hill_climbing': {'color': '#1f77b4', 'marker': 's', 'label': 'Hill Climbing'},
        'local_search': {'color': '#2ca02c', 'marker': '^', 'label': 'Local Search'},
        'lp_tkr': {'color': '#ff7f0e', 'marker': 'x', 'label': 'LP TKR'},
        'lp_irp': {'color': '#9467bd', 'marker': 'd', 'label': 'LP IRP'},
        'pg_gaurd_neigh': {'color': '#8c564b', 'marker': '*', 'label': 'PG Guard Neigh'},
        'pg_sort_topk': {'color': '#e377c2', 'marker': 'p', 'label': 'PG Sort TopK'}
    }

    # -- Subplot 1: % Nodes Saved --
    for col, style in styles.items():
        if col in df_nodes.columns:
            sub = df_nodes.dropna(subset=[col])
            if not sub.empty:
                # x-axis is log2 of number of nodes
                ax1.plot(np.log2(sub['num_nodes']), sub[col], 
                         label=style['label'], color=style['color'], marker=style['marker'])
                                
    ax1.set_ylabel('% Nodes Saved')
    ax1.set_title('Percentage of Nodes Saved (Step Size = 3)')
    ax1.grid(True, linestyle='--', alpha=0.7)
    ax1.legend(loc='best')

    # -- Subplot 2: Log Time Taken --
    for col, style in styles.items():
        if col in df_time.columns:
            sub = df_time.dropna(subset=[col])
            if not sub.empty:
                # Plot the full line (solid)
                ax2.plot(np.log2(sub['num_nodes']), sub[col], 
                         color=style['color'], linestyle='-', alpha=0.6)
                
                # Handle markers: solid for real, hollow for extrapolated
                if col in extrapolated_points:
                    extra_nodes = [p[0] for p in extrapolated_points[col]]
                    real_sub = sub[~sub['num_nodes'].isin(extra_nodes)]
                    extra_sub = sub[sub['num_nodes'].isin(extra_nodes)]
                    
                    # Plot real points
                    ax2.plot(np.log2(real_sub['num_nodes']), real_sub[col], 
                             color=style['color'], marker=style['marker'], linestyle='', label=style['label'])
                    
                    # Plot extrapolated points (hollow circles)
                    if not extra_sub.empty:
                        ax2.plot(np.log2(extra_sub['num_nodes']), extra_sub[col], 
                                 color=style['color'], marker='o', linestyle='', 
                                 markerfacecolor='white', markeredgewidth=1.5, label=None)
                else:
                    # Normal plotting for algorithms without extrapolation
                    ax2.plot(np.log2(sub['num_nodes']), sub[col], 
                             label=style['label'], color=style['color'], marker=style['marker'])

    ax2.set_ylabel('Time (s) [Log Scale]')
    ax2.set_yscale('log')
    ax2.set_xlabel('Number of Nodes ($log_2$)')
    ax2.set_title('Time Taken vs Nodes (Extrapolated points shown as hollow circles)')
    ax2.grid(True, linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    plt.savefig('benchmark_plot.png')
    print("Plot saved as benchmark_plot.png")
    plt.show()

if __name__ == "__main__":
    generate_benchmark_plot()