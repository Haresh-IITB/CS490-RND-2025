#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <iomanip> // for setprecision
#include <sys/stat.h> // For mkdir
#include "waxman-graph.h" 
#include<sstream>
// Utility to create directories
void ensure_directory(const std::string& path) {
    std::string cmd = "mkdir -p " + path;
    system(cmd.c_str());
}

// Helper to save graph nodes and edges to CSV
void save_graph_snapshot(const Graph& g, const std::string& prefix) {
    // Save Nodes
    std::ofstream nf(prefix + "_nodes.csv");
    nf << "id,x,y,tier\n";
    for (const auto& n : g.nodes) {
        nf << n.id << "," << n.x << "," << n.y << "," << n.tier << "\n";
    }
    nf.close();

    // Save Edges
    std::ofstream ef(prefix + "_edges.csv");
    ef << "u,v\n";
    for (int u = 0; u < (int)g.adj_list.size(); ++u) {
        for (int v : g.adj_list[u]) {
            if (u < v) { // Undirected, save once
                ef << u << "," << v << "\n";
            }
        }
    }
    ef.close();
}

void run_parameter_experiment(const std::string& out_dir) {
    std::cout << "[Task 1] Running Parameter Variations (2x2 Grid)...\n";
    
    // Define 4 configs for a 2x2 grid
    struct Config { double a, b; };
    std::vector<Config> configs = {
        {0.05, 0.05}, {0.05, 0.3},  // Row 1: Low Alpha (Sparse) varying Beta
        {0.3, 0.05},  {0.3, 0.3}    // Row 2: High Alpha (Dense) varying Beta
    };

    Graph::Params p;
    // Fixed sizes for comparison
    p.num_t1 = 2; p.nodes_per_t1 = 40;
    p.num_t2 = 3; p.nodes_per_t2 = 20;
    p.num_t3 = 5; p.nodes_per_t3 = 10;
    p.num_villages = 5; p.nodes_per_village = 5;
    p.cutoff_prob = 1e-3;

    for(const auto& cfg : configs) {
        p.alpha = cfg.a;
        p.beta = cfg.b;
        
        // Initialize Graph with fixed seed for reproducibility
        Graph g(p, 12345); 
        g.generate_centers();
        g.generate_nodes();
        g.build_spatial_index();
        g.generate_edges();
        
        // Create filename like "alpha_0.05_beta_0.1"
        std::stringstream ss;
        ss << out_dir << "/alpha_" << std::fixed << std::setprecision(2) << cfg.a 
           << "_beta_" << std::setprecision(2) << cfg.b;
        
        save_graph_snapshot(g, ss.str());
    }
}

void run_movement_experiment(const std::string& out_dir) {
    std::cout << "[Task 1] Running Movement Dynamics (Dense Graph)...\n";
    
    Graph::Params p;
    // Dense configuration
    p.num_t1 = 3;   p.nodes_per_t1 = 100;
    p.num_t2 = 6;   p.nodes_per_t2 = 40;
    p.num_t3 = 12;  p.nodes_per_t3 = 15;
    p.num_villages = 20; p.nodes_per_village = 5;
    
    p.alpha = 0.2; p.beta = 0.1; // Reasonable connectivity
    p.cutoff_prob = 1e-3;

    Graph g(p, 999);
    g.generate_centers();
    g.generate_nodes();
    g.build_spatial_index();
    g.generate_edges();

    std::cout << "Generated dense graph with " << g.nodes.size() << " nodes.\n";

    // Save Initial State
    save_graph_snapshot(g, out_dir + "/move_start");

    // Simulate Movement (30 steps to see significant shift)
    g.simulate_movement(30);

    // Save Final State
    save_graph_snapshot(g, out_dir + "/move_end");
}

void run_time_analysis(const std::string& out_dir) {
    std::cout << "[Task 1] Running Time Analysis (Extended N)...\n";
    
    std::ofstream time_file(out_dir + "/time_analysis.csv");
    time_file << "N,Time_ms\n";

    // Extended range of nodes
    std::vector<int> node_counts = {100,500,1000, 2000, 5000, 10000, 20000};
    
    Graph::Params p;
    // Simplified structure for pure scaling test (mostly T1 nodes)
    p.num_t1 = 1; 
    p.num_t2 = 0; p.num_t3 = 0; p.num_villages = 0; 
    p.alpha = 0.1; p.beta = 0.1;
    p.cutoff_prob = 1e-3;

    for (int n : node_counts) {
        p.nodes_per_t1 = n; 

        auto start = std::chrono::high_resolution_clock::now();

        Graph g(p, 42);
        g.generate_centers();
        g.generate_nodes();
        g.build_spatial_index();
        g.generate_edges();

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        
        time_file << g.nodes.size() << "," << elapsed.count() << "\n";
        std::cout << "Generated N=" << g.nodes.size() << " in " << elapsed.count() << " ms\n";
    }
    time_file.close();
}

int main() {
    std::string out_dir = "results/graph-results";
    ensure_directory(out_dir);

    run_parameter_experiment(out_dir);
    run_movement_experiment(out_dir);
    run_time_analysis(out_dir);

    std::cout << "Task 1 Complete. Results saved to " << out_dir << "\n";
    return 0;
}