import subprocess
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
import time

# --- Configuration ---
TIMER_EXECUTABLE = "./bin/timer"
OUTPUT_CSV = "results/time_analysis_results.csv"
OUTPUT_PLOT = "results/visualizations/time_analysis_plot.png"

# List of N values to test.
# Start small, get larger.
# 50k+ nodes can take a few seconds each.
N_VALUES = [
    1000, 2000, 5000,
    10000, 15000, 20000,
    30000, 40000, 50000
]
# ---------------------

def run_benchmark():
    """
    Runs the C++ timer executable for all N_VALUES and collects the results.
    """
    if not os.path.exists(TIMER_EXECUTABLE):
        print(f"Error: Timer executable '{TIMER_EXECUTABLE}' not found.")
        print("Please compile the project first using 'make'.")
        return None

    results = []
    for n in N_VALUES:
        print(f"Running benchmark for N = {n}...")
        start_time = time.time()
        
        try:
            # Run the C++ timer process
            result = subprocess.run(
                [TIMER_EXECUTABLE, str(n)],
                capture_output=True,
                text=True,
                check=True,
                timeout=60  # 60-second timeout per run
            )
            
            # Parse the CSV output (e.g., "50000,1234")
            n_out, time_ms = result.stdout.strip().split(',')
            
            # Sanity check
            if int(n_out) != n:
                print(f"Warning: Mismatch in N (Expected {n}, got {n_out})")
                
            results.append({'N': int(n_out), 'Time_ms': int(time_ms)})
            
            run_duration = (time.time() - start_time) * 1000
            print(f"  -> C++ reported: {time_ms} ms")
            
        except subprocess.TimeoutExpired:
            print(f"  -> FAILED (Timeout after 60s)")
        except Exception as e:
            print(f"  -> FAILED ({e})")
            print(f"  -> Stderr: {e.stderr if hasattr(e, 'stderr') else 'N/A'}")

    return pd.DataFrame(results)

def plot_results(df):
    """
    Plots the benchmark results and fits an O(N log N) curve.
    """
    if df.empty:
        print("No data to plot.")
        return

    print("Plotting results...")
    
    # Create the O(N log N) term for fitting
    # We add a small epsilon to avoid log(0) if N=0
    df['N_log_N'] = df['N'] * np.log(df['N'] + 1e-9)

    # Fit a linear model (y = a*x)
    # y = Time_ms, x = N_log_N
    # This finds the 'a' in Time = a * (N log N)
    
    # Using numpy.linalg.lstsq for a simple linear fit
    A = df[['N_log_N']]
    b = df['Time_ms']
    
    # `coef` will be our 'a'
    coef, _, _, _ = np.linalg.lstsq(A, b, rcond=None)
    a = coef[0]
    
    # Create the fitted curve
    df['Fit_N_log_N'] = a * df['N_log_N']

    # --- Plotting ---
    plt.figure(figsize=(10, 6))
    
    # 1. Plot the measured data points
    plt.scatter(df['N'], df['Time_ms'], 
                label='Measured Time (Actual)', color='blue', zorder=2)
    
    # 2. Plot the O(N log N) fitted curve
    plt.plot(df['N'], df['Fit_N_log_N'], 
             label=f'Fitted Curve: O(N log N)\n(y = {a:.4f} * N log N)', 
             color='red', linestyle='--', zorder=1)

    plt.title('Graph Generation Time Complexity', fontsize=16)
    plt.xlabel('Number of Nodes (N)', fontsize=12)
    plt.ylabel('Time to Generate (milliseconds)', fontsize=12)
    plt.legend()
    plt.grid(True, which='both', linestyle=':', linewidth=0.5)
    
    # Use scientific notation for large N if needed
    plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
    
    # Ensure the output directory exists
    os.makedirs("results/visualizations", exist_ok=True)
    
    plt.savefig(OUTPUT_PLOT)
    print(f"\nSuccessfully saved time analysis plot to:\n{OUTPUT_PLOT}")

# --- Main execution ---
if __name__ == "__main__":
    df_results = run_benchmark()
    if df_results is not None and not df_results.empty:
        df_results.to_csv(OUTPUT_CSV, index=False)
        print(f"\nBenchmark results saved to:\n{OUTPUT_CSV}")
        plot_results(df_results)
    else:
        print("\nBenchmark run failed or produced no data.")