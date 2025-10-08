#include <iostream>
#include <vector>
#include <functional>
#include <set>
#include <fstream>
#include "waxman-graph.h"
#include "Random_number_generator.h"
#include "strategy.h"
#include "simulation.h"


int main() {

    int num_nodes = 500;      // Total nodes in the graph
    double alpha = 0.4;       // Waxman alpha parameter (controls edge density)
    double beta  = 0.1;       // Waxman beta parameter (controls edge length preference)
    int k_budget = 15;         // Number of nodes we can vaccinate
    int num_infected = 20;    // Number of initially infected nodes

    std::cout << "Parameters:\n";
    std::cout << "  - Nodes: " << num_nodes << "\n";
    std::cout << "  - Vaccination Budget: " << k_budget << "\n";
    std::cout << "  - Initially Infected: " << num_infected << "\n";
    std::cout << "---------------------------\n\n";

    Random_number_generator rng;

    // --- Graph Generation ---
    std::cout << "Generating Waxman graph \n";
    Graph g(num_nodes);
    g.generate_nodes();
    g.generate_edges(alpha, beta);
    std::cout << "Graph generated with " << g.V << " actual nodes.\n\n";

    num_nodes = g.V;

    // ---  Setup Initial Infection ---
    std::cout << "Selecting initial infected nodes \n";
    std::vector<int> infected_nodes;
    std::set<int> infected_set;

    while (infected_set.size() < static_cast<size_t>(num_infected)) {
        int infected_node = static_cast<int>(rng.get_unif(0, num_nodes));
        infected_set.insert(infected_node);
    }

    // Save infected nodes to infected.csv
    std::ofstream infected_file("results/infected.csv");
    infected_file << "node_id\n";
    std::cout << "Infected nodes are: ";
    for (int node : infected_set) {
        infected_nodes.push_back(node);
        std::cout << node << " ";
        infected_file << node << "\n";
    }
    infected_file.close();
    std::cout << "\n\n";


    // --- Run Vaccination Algorithm ---
    std::cout << "Running vaccination algorithm to find best " << k_budget << " nodes to vaccinate\n";

    std::vector<int> nodes_to_vaccinate = greedy_vaccination(g, k_budget, infected_nodes,ltSimulation);
    // std::vector<int> nodes_to_vaccinate = Local_search(g, k_budget, infected_nodes, IC_Simulation);

    // Save vaccinated nodes to vaccinated.csv
    std::ofstream vaccinated_file("results/vaccinated.csv");
    vaccinated_file << "node_id\n";
    std::cout << "Vaccination algorithm suggests vaccinating nodes: ";
    for (int node : nodes_to_vaccinate) {
        std::cout << node << " ";
        vaccinated_file << node << "\n";
    }
    vaccinated_file.close();
    std::cout << "\n\n";

    // ---  Evaluate and Compare Results ---
    std::cout << "Step 4: Evaluating the impact of vaccination...\n";
    std::vector<bool> isVaccinable(num_nodes, true);
    for(int node : infected_nodes) {
        isVaccinable[node] = false;
    }

    // Run simulation WITHOUT vaccination
    int saved_without_vaccination = ltSimulation(g, isVaccinable, -1, infected_nodes);
    std::cout << "Result without vaccination: " << saved_without_vaccination << " nodes were saved.\n";

    for(int node : nodes_to_vaccinate) {
        isVaccinable[node] = false;
    }
    
    // Run simulation WITH vaccination
    int saved_with_vaccination = ltSimulation(g, isVaccinable, -1, infected_nodes);
    std::cout << "Result with vaccination:    " << saved_with_vaccination << " nodes were saved.\n";

    int extra_saved = saved_with_vaccination - saved_without_vaccination;
    std::cout << "Vaccination saved an additional " << extra_saved << " nodes.\n\n";

    // Save the final results to results.csv
    std::ofstream results_file("results/results.csv");
    results_file << "category,saved_nodes\n";
    results_file << "Without Vaccination," << saved_without_vaccination << "\n";
    results_file << "With Vaccination," << saved_with_vaccination << "\n";
    results_file.close();


    // --- Save Graph for Visualization ---
    std::cout << "Step 5: Saving graph data to CSV files...\n";
    g.save_nodes_to_file("results/graph/nodes.csv", "results/graph/edges.csv");
    std::cout << "\n--- Simulation Complete ---\n";
    std::cout << "You can now run 'visualize.py' to see the graph.\n";

    return 0;
}

