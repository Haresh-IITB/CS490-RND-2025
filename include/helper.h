#ifndef HELPER_H
#define HELPER_H

#include <vector>
#include <functional>
#include <fstream>
#include "waxman-graph.h"
#include "simulation.h"


int evaluate_solution(
    Graph& g,
    const std::vector<int>& vaccinated_nodes,
    const std::vector<int>& infected_nodes,
    std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int>&)> evaluator
);


std::vector<int> run_single_experiment(
    Graph& g,
    int k_budget,
    int k_percent,
    int num_infected,
    int infected_percent,
    const std::vector<int>& infected_nodes,
    std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int>&)> evaluator,
    std::ofstream& results_file
);

#endif 