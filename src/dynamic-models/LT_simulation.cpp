#include "waxman-graph.h"
#include <queue>
#include <iostream>
#include <iomanip>

void simulate_infection_spread_LT(
    const Graph &G, 
    std::vector<int> &infected_list, 
    std::vector<bool> &infected_mask,
    const std::vector<bool> &vaccinated_mask, 
    int steps, 
    uint64_t seed,
    double prob_infect
) {
    const double P = prob_infect ; // Infection probability factor
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uni(0.0, 1.0);
    
    std::vector<double> influence(G.nodes.size(), 0.0);
    // Intitalise using the infected_mask 
    for(int i = 0 ; i< (int)infected_mask.size(); ++i){
        if(infected_mask[i]){
            for(int v : G.adj_list[i]){
                double w = 3.0 / std::max(1, (int)G.adj_list[v].size());
                influence[v] += w ;
            }
        }
    }

    std::vector<int> frontier = infected_list;

    for (int t = 0; t < steps; ++t) {
        std::vector<int> next_frontier;
        for (int u : frontier) {
            for (int v : G.adj_list[u]) {
                // If v is susceptible (not infected, not vaccinated)
                if (!infected_mask[v] && !vaccinated_mask[v]) {
                    double w = 1.0 / std::max(1, (int)G.adj_list[v].size());
                    influence[v] += w ;

                    // Check threshold
                    if (influence[v] >= G.nodesThreshold[v] && P > uni(rng)) {
                        infected_mask[v] = true ;
                        next_frontier.push_back(v);
                    }
                }                
            }
        }
        frontier = next_frontier;
        if (frontier.empty()) break;
    }

    // Update the master list
    infected_list.assign(frontier.begin(), frontier.end()); // Contains all the infected nodes after simulation
}

// *************************************************************************************************************************
// This one is for testing 
// *************************************************************************************************************************
int LT_Simulation_test(
    Graph &G,
    const std::vector<std::pair<int,int>> &Vaccinated_Node, // (node, time_id)
    const std::vector<bool> &Infected_Node,
    const uint64_t &seed,
    const int &stepSize,
    const double prob_infect
) {
    // Copy graph
    Graph::Params params_copy = G.params;
    Graph Gc(params_copy, seed);

    Gc.centers = G.centers;
    Gc.nodes = G.nodes;
    Gc.adj_list = G.adj_list;
    Gc.nodesThreshold = G.nodesThreshold;
    Gc.build_spatial_index();

    const int N = Gc.nodes.size();

    // LT state
    std::vector<bool> active(N, false);
    std::vector<double> influence(N, 0.0);

    // Immune mask
    std::vector<int> immune(N, 0);
    int immune_ptr = 0;
    int time_id = 0;
    // std::cout << "Total scheduled vaccinations: " << Vaccinated_Node.size() << std::endl;       
    while (immune_ptr < (int)Vaccinated_Node.size() &&
           Vaccinated_Node[immune_ptr].second == time_id) {
        immune[Vaccinated_Node[immune_ptr].first] = 1;
        immune_ptr++;
    }

    // Initial active nodes
    std::queue<int> frontier;
    int activeCount = 0;

    for (int u = 0; u < N; ++u) {
        if (Infected_Node[u] && !immune[u]) {
            active[u] = true;
            frontier.push(u);
            activeCount++;
        }
    }

    int steps = 0;
    int time = 0;

    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uni(0.0, 1.0);

    constexpr double INFLUENCE_SCALE = 3.0;

    while (!frontier.empty()) {
        // std::cout << "Step " << steps << " | Frontier Size: " << frontier.size() << " | Active Count: " << activeCount << " Vaccine Count : "<< std::count(immune.begin(), immune.end(), 1) << std::endl;
        // Apply scheduled vaccinations
        while (immune_ptr < (int)Vaccinated_Node.size() &&
               Vaccinated_Node[immune_ptr].second == time_id) {
            immune[Vaccinated_Node[immune_ptr].first] = 1;
            immune_ptr++;
        }

        std::queue<int> next_frontier;

        // Movement (IC-consistent)
        if (steps > 0 && stepSize > 0 && steps % stepSize == 0) {
            Gc.simulate_movement(stepSize);
            time++;
        }

        while (!frontier.empty()) {
            int u = frontier.front();
            frontier.pop();

            for (int v : Gc.adj_list[u]) {
                if (active[v] || immune[v]) continue;

                double w =
                    INFLUENCE_SCALE / std::max(1, (int)Gc.adj_list[v].size());

                influence[v] += w;

                if (influence[v] >= Gc.nodesThreshold[v] && prob_infect > uni(rng)) {
                    active[v] = true;
                    next_frontier.push(v);
                    activeCount++;
                }
            }
        }

        frontier = next_frontier;
        steps++;
        time_id++;
    }

    return N - activeCount;
}

int LT_Simulation_search(
    Graph &G,
    const std::vector<bool> &Vaccinated_Node,
    const int &newVaccinatedNode,
    const std::vector<bool> &Infected_Node,
    const std::vector<int> &current_infected,
    const uint64_t &seed,
    const int &stepSize,
    const double prob_infect
) {
    // Copy graph
    Graph::Params params_copy = G.params;
    Graph Gc(params_copy, seed);

    Gc.centers = G.centers;
    Gc.nodes = G.nodes;
    Gc.adj_list = G.adj_list;
    Gc.nodesThreshold = G.nodesThreshold;
    Gc.build_spatial_index();

    const int N = Gc.nodes.size();

    std::vector<bool> active = Infected_Node;
    std::vector<double> influence(N, 0.0);
    std::vector<bool> immune = Vaccinated_Node;
    if (newVaccinatedNode != -1)
        immune[newVaccinatedNode] = true;


    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uni(0.0, 1.0);

    // Initial active nodes
    std::queue<int> frontier;
    int activeCount = std::count(active.begin(), active.end(), true);

    for(int u : current_infected) {
        if (!active[u]) {
            active[u] = true;
            activeCount++;
        }
        frontier.push(u);
    }

    int steps = 0;

    constexpr double INFLUENCE_SCALE = 3.0;

    while (!frontier.empty()) {
        std::queue<int> next_frontier;

        // Movement (IC-consistent)
        if (steps > 0 && stepSize > 0 && steps % stepSize == 0) {
            Gc.simulate_movement(stepSize);
        }

        while (!frontier.empty()) {
            int u = frontier.front();
            frontier.pop();

            for (int v : Gc.adj_list[u]) {
                if (active[v] || immune[v]) continue;

                double w =
                    INFLUENCE_SCALE / std::max(1, (int)Gc.adj_list[v].size());

                influence[v] += w;

                if (influence[v] >= Gc.nodesThreshold[v] && prob_infect > uni(rng)) {
                    active[v] = true;
                    next_frontier.push(v);
                    activeCount++;
                }
            }
        }

        frontier = next_frontier;
        steps++;
    }

    return N - activeCount;
}