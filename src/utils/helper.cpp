#include "helper.h"
#include "strategy.h" // Assumed to contain declarations for the algorithms
#include <iostream>
#include <chrono>

int evaluate_solution(
    Graph& g,
    const std::vector<int>& vaccinated_nodes,
    const std::vector<int>& infected_nodes,
    std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int>&)> evaluator)
{
    std::vector<bool> is_vaccinable(g.V, true);
    for (int node : infected_nodes) {
        is_vaccinable[node] = false;
    }
    for (int node : vaccinated_nodes) {
        is_vaccinable[node] = false;
    }

    return evaluator(g, is_vaccinable, -1, infected_nodes);
}

std::vector<int> run_single_experiment(
    Graph& g,
    int k_budget,
    int k_percent,
    int num_infected,
    int infected_percent,
    const std::vector<int>& infected_nodes,
    std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int>&)> evaluator,
    std::ofstream& results_file)
{
    int actual_num_nodes = g.V;

    // --- Greedy ---
    std::cout << "  Running Greedy..." << std::flush;
    auto start_greedy = std::chrono::high_resolution_clock::now();
    std::vector<int> vaccinated_greedy = greedy_vaccination(g, k_budget, infected_nodes, evaluator);
    auto end_greedy = std::chrono::high_resolution_clock::now();
    long long duration_greedy = std::chrono::duration_cast<std::chrono::milliseconds>(end_greedy - start_greedy).count();
    int saved_greedy = evaluate_solution(g, vaccinated_greedy, infected_nodes, evaluator);
    results_file << actual_num_nodes << "," << k_percent << "," << infected_percent << ",Greedy," << saved_greedy << "," << duration_greedy << "\n";
    std::cout << " Done. (Saved: " << saved_greedy << ", Time: " << duration_greedy << "ms)\n";

    // --- Local Search ---
    std::cout << "  Running Local Search..." << std::flush;
    auto start_ls = std::chrono::high_resolution_clock::now();
    std::vector<int> vaccinated_ls = Local_search(g, k_budget, infected_nodes, evaluator);
    auto end_ls = std::chrono::high_resolution_clock::now();
    long long duration_ls = std::chrono::duration_cast<std::chrono::milliseconds>(end_ls - start_ls).count();
    int saved_ls = evaluate_solution(g, vaccinated_ls, infected_nodes, evaluator);
    results_file << actual_num_nodes << "," << k_percent << "," << infected_percent << ",Local Search," << saved_ls << "," << duration_ls << "\n";
    std::cout << " Done. (Saved: " << saved_ls << ", Time: " << duration_ls << "ms)\n";

    // --- Hill Climbing ---
    std::cout << "  Running Hill Climbing..." << std::flush;
    auto start_hc = std::chrono::high_resolution_clock::now();
    std::vector<int> vaccinated_hc = hill_climbing(g, k_budget, infected_nodes, evaluator, vaccinated_greedy);
    auto end_hc = std::chrono::high_resolution_clock::now();
    long long duration_hc = std::chrono::duration_cast<std::chrono::milliseconds>(end_hc - start_hc).count();
    int saved_hc = evaluate_solution(g, vaccinated_hc, infected_nodes, evaluator);
    results_file << actual_num_nodes << "," << k_percent << "," << infected_percent << ",Hill Climbing," << saved_hc << "," << duration_hc << "\n";
    std::cout << " Done. (Saved: " << saved_hc << ", Time: " << duration_hc << "ms)\n";

    return vaccinated_hc;
}