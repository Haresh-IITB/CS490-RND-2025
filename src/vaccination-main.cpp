#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <map>
#include <algorithm>
#include <numeric>

// Use your specific headers
#include "waxman-graph.h"
#include "simulation.h"
#include "strategy.h"

// --- Helper to ensure directory exists ---
void ensure_dir(const std::string& path) {
    std::string cmd = "mkdir -p " + path;
    system(cmd.c_str());
}

// --- Configuration Struct ---
struct Config {
    // Benchmark ranges
    std::vector<int> graph_sizes = {100, 200, 300, 400, 500}; 
    std::vector<int> budgets_percent = {5, 10, 15}; 
    
    // Graph Params
    double alpha = 0.2;
    double beta = 0.1;
    
    // Epidemic Params
    double initial_infected_pct = 2.0;
};

int main() {
    std::string out_dir = "results/task2_analysis";
    ensure_dir(out_dir);
    std::string result_file = out_dir + "/benchmark_results.csv";
    
    std::ofstream csv(result_file);
    csv << "Model,Algorithm,Nodes_N,Budget_Percent,Budget_K,Time_ms,Nodes_Saved\n";

    Config cfg; 

    // Models to test
    std::vector<std::string> models = {"IC", "LT"};
    
    // Algorithms to test
    std::vector<std::string> algos = {"Greedy", "LocalSearch", "HillClimbing", "PageRank"};

    std::cout << "========================================\n";
    std::cout << "   STARTING VACCINATION BENCHMARK       \n";
    std::cout << "========================================\n";

    // 1. Loop over Graph Sizes
    for (int N : cfg.graph_sizes) {
        std::cout << "\n>>> Generating Graph N=" << N << "...\n";
        
        // Construct Graph Params dynamically based on N
        Graph::Params p;
        p.nodes_per_t1 = N; // Simplified scaling: put all nodes in T1 to control exact count
        p.num_t1 = 1; p.num_t2=0; p.num_t3=0; p.num_villages=0;
        p.alpha = cfg.alpha; p.beta = cfg.beta;
        p.cutoff_prob = 1e-3;
        
        Graph G(p, 12345); // Fixed seed
        G.generate_centers(); G.generate_nodes();
        G.build_spatial_index(); G.generate_edges();

        // Pick Initial Infected
        int num_infected = std::max(1, (int)(N * cfg.initial_infected_pct / 100.0));
        std::vector<int> infected_nodes;
        std::vector<int> pool(N);
        std::iota(pool.begin(), pool.end(), 0);
        std::shuffle(pool.begin(), pool.end(), G.rng_gen->rng);
        for(int i=0; i<num_infected; ++i) infected_nodes.push_back(pool[i]);

        // 2. Loop over Budget Percentages
        for (int pct : cfg.budgets_percent) {
            int K = std::max(1, (int)(N * pct / 100.0));
            std::cout << "   [Budget " << pct << "% (K=" << K << ")]\n";

            // 3. Loop over Models (IC/LT)
            for (const auto& model_name : models) {
                
                // Define Evaluator Lambda
                auto evaluator = [&](Graph& g, std::vector<bool>& v, const int& n, const std::vector<int>& i) {
                    if (model_name == "IC") return IC_Simulation(g, v, n, i);
                    else return LT_Simulation(g, v, n, i);
                };

                // 4. Loop over Algorithms
                for (const auto& algo : algos) {
                    // Optimization: Skip Greedy on larger graphs if it's too slow for quick testing
                    if (N > 400 && algo == "Greedy") { 
                        // continue; 
                    }

                    auto start = std::chrono::high_resolution_clock::now();
                    std::vector<int> vaccinated;

                    if (algo == "Greedy") {
                        vaccinated = greedy_vaccination(G, K, infected_nodes, evaluator);
                    } 
                    else if (algo == "LocalSearch") {
                        vaccinated = Local_search(G, K, infected_nodes, evaluator, 50); // max_iter=50
                    }
                    else if (algo == "PageRank") {
                        // Matches strategy.h signature
                        vaccinated = PageRank(G, K, infected_nodes, 0.85, 1e-6, 50);
                    }
                    else if (algo == "HillClimbing") {
                        // Hill Climbing requires an initial set. 
                        // We use PageRank to generate a smart initial seed.
                        std::vector<int> seed = PageRank(G, K, infected_nodes, 0.85, 1e-6, 50); 
                        vaccinated = hill_climbing(G, K, infected_nodes, evaluator, seed, 50);
                    }

                    auto end = std::chrono::high_resolution_clock::now();
                    double time_ms = std::chrono::duration<double, std::milli>(end - start).count();

                    // Final Evaluation of Strategy
                    // Create the mask: true = vaccinable (not vaccinated), false = vaccinated
                    std::vector<bool> mask(N, true);
                    for(int v : vaccinated) mask[v] = false;
                    
                    // Run simulation to get final saved count
                    int saved = evaluator(G, mask, -1, infected_nodes);

                    // Log to Console
                    std::cout << "      " << std::left << std::setw(14) << algo 
                              << "| Model: " << model_name 
                              << "| Time: " << std::setw(6) << (int)time_ms << "ms"
                              << "| Saved: " << saved << "/" << N << "\n";

                    // Log to CSV
                    csv << model_name << "," << algo << "," << N << "," << pct << "," << K << "," 
                        << time_ms << "," << saved << "\n";
                    csv.flush();
                }
            }
        }
    }

    csv.close();
    std::cout << "\nBenchmark completed. Data saved to " << result_file << "\n";
    return 0;
}