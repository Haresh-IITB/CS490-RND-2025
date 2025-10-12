#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <set>
#include <cmath>
#include <functional>
#include "config.h"
#include "waxman-graph.h"
#include "Random_number_generator.h"
#include "simulation.h"
#include "helper.h"

int main(int argc, char * argv[]) {
    
    if (argc != 2) {
        std::cout << "Usage: ./simulation config.txt" << std::endl;
        return 1;
    }

    Config parameters = initialiseConfig(std::string(argv[1]));
    std::cout << "Configuration loaded successfully." << std::endl;

    // Select Simulation Model based on Config
    std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int>&)> evaluator;
    if (parameters.model == "IC") {
        evaluator = IC_Simulation;
        std::cout << "Model selected: Independent Cascade (IC)" << std::endl;
    } else {
        evaluator = ltSimulation; // Default to LT
        std::cout << "Model selected: Linear Threshold (LT)" << std::endl;
    }
    
    std::ofstream results_file("results/experiment_results.csv");
    results_file << "NumNodes,K_Percent,Infected_Percent,Algorithm,NodesSaved,TimeTaken_ms\n";

    Random_number_generator rng;
    Graph g_final;
    std::vector<int> last_infected_nodes;
    std::vector<int> last_vaccinated_nodes;

    for (int k_percent : parameters.k_budget) {
        for (int infected_percent : parameters.infected) {
            for (int num_nodes_config : parameters.nodes) {
                
                std::cout << "\n";
                std::cout << "Starting Experiment: Nodes=" << num_nodes_config 
                          << ", k=" << k_percent << "%, infected=" << infected_percent << "%" << std::endl;
                std::cout << "\n";

                Graph current_g(num_nodes_config);
                current_g.generate_nodes();
                current_g.generate_edges(parameters.alpha, parameters.beta);
                
                int k_budget = static_cast<int>(round((k_percent / 100.0) * current_g.V));
                int num_infected = static_cast<int>(round((infected_percent / 100.0) * current_g.V));

                std::vector<int> infected_nodes;
                std::set<int> infected_set;
                while (infected_set.size() < static_cast<size_t>(num_infected)) {
                    int node = static_cast<int>(rng.get_unif(0, current_g.V));
                    infected_set.insert(node);
                }
                for (int node : infected_set) infected_nodes.push_back(node);
                
                std::vector<int> vaccinated_nodes_result = run_single_experiment(
                    current_g, k_budget, k_percent, num_infected, 
                    infected_percent, infected_nodes, evaluator, results_file
                );

                // Check if this is the final experiment to save its state for visualization
                if (k_percent == parameters.k_budget.back() && 
                    infected_percent == parameters.infected.back() && 
                    num_nodes_config == parameters.nodes.back()) {
                    
                    g_final = current_g;
                    last_infected_nodes = infected_nodes;
                    last_vaccinated_nodes = vaccinated_nodes_result;
                }
            }
        }
    }

    results_file.close();
    std::cout << "\nAll experiments finished. Results saved to experiment_results.csv" << std::endl;

    // Save data from the FINAL experiment for visualization
    std::cout << "Saving data from the last run for visualization..." << std::flush;
    
    g_final.save_nodes_to_file("results/graph/nodes.csv", "results/graph/edges.csv");

    std::ofstream infected_file("results/infected.csv");
    infected_file << "node_id\n";
    for(int node : last_infected_nodes) infected_file << node << "\n";
    infected_file.close();

    std::ofstream vaccinated_file("results/vaccinated.csv");
    vaccinated_file << "node_id\n";
    for(int node : last_vaccinated_nodes) vaccinated_file << node << "\n";
    vaccinated_file.close();

    std::cout << " Done." << std::endl;

    return 0;
}