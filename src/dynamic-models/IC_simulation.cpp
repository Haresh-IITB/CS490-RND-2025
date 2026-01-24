#include "waxman-graph.h"
#include <queue>
#include <iostream>
#include <iomanip>

void simulate_infection_spread_IC(
    const Graph &G, 
    std::vector<int> &infected_list, // Input/Output
    std::vector<bool> &infected_mask,
    const std::vector<bool> &vaccinated_mask, 
    int steps, 
    uint64_t seed,
    double prob_infect
) {
    const double P = prob_infect ; // Infection probability factor
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uni(0.0, 1.0);
    
    for(int u : infected_list)
        infected_mask[u] = true ;

    std::vector<int> frontier = infected_list;

    for (int t = 0; t < steps; ++t) {
        std::vector<int> next_frontier;
        for (int u : frontier) {
            for (int v : G.adj_list[u]) {
                // If v is susceptible (not infected, not vaccinated)
                if (!infected_mask[v] && !vaccinated_mask[v]) {
                    // Check transmission prob
                    if (P > uni(rng)) {
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
    infected_list.assign(frontier.begin(), frontier.end()); // Contains all the infected nodes after simulation that act as next frontier 
}

// *************************************************************************************************************************
// This one is for testing 
// *************************************************************************************************************************
int IC_Simulation_test(Graph &G,
                  const std::vector<std::pair<int,int>> &Vaccinated_Node,
                  const std::vector<bool> &Infected_Node,
                  const uint64_t &seed,
                  const int &stepSize,
                  const double prob_infect)
{
    // 1) Copy the graph
    Graph::Params params_copy = G.params;
    Graph Gc(params_copy, seed);

    Gc.centers = G.centers;
    Gc.nodes = G.nodes;
    Gc.adj_list = G.adj_list;
    Gc.nodesThreshold = G.nodesThreshold;
    Gc.build_spatial_index();

    UndirectedEdgeRandom edgeRand(seed);

    const int N = Gc.nodes.size();

    int immune_ptr = 0, time_id = 0 ; 

    // Immune mask
    std::vector<int> immune(N, 0);
    while (immune_ptr < (int)Vaccinated_Node.size() && Vaccinated_Node[immune_ptr].second == time_id){
        immune[Vaccinated_Node[immune_ptr].first] = 1 ; 
        immune_ptr ++ ; 
    }

    // Initial infection
    std::vector<bool> infected(N, 0);
    std::queue<int> frontier;
    int infectedCount = 0;

    for (int u = 0; u < N; ++u) {
        if (Infected_Node[u]) {
            infected[u] = 1;
            frontier.push(u);
            infectedCount++;
        }
    }

    int steps = 0;
    int time = 0 ; 
    // IC propagation

    while (!frontier.empty()) {

        // Distribute the vaccines 
        while(immune_ptr < Vaccinated_Node.size() && Vaccinated_Node[immune_ptr].second == time_id){
            immune[Vaccinated_Node[immune_ptr].first] = 1 ; 
            immune_ptr ++ ; 
        }

        std::queue<int> next_frontier;
        if (steps > 0 && stepSize > 0 && steps % stepSize == 0) {
            Gc.simulate_movement(stepSize);
            time ++ ; 
        }

        while (!frontier.empty()) {
            int u = frontier.front();
            frontier.pop();

            for (int v : Gc.adj_list[u]) {
                if (infected[v] || immune[v]) continue;
                double r = edgeRand.get(u, v, time);
                if (r < prob_infect) {
                    infected[v] = 1;
                    next_frontier.push(v);
                    infectedCount++;
                }
            }
        }

        frontier = next_frontier;
        steps++;
        time_id ++ ; 
    }

    return N - infectedCount;
}

int IC_Simulation_search(Graph &G,
                  const std::vector<bool> &Vaccinated_Node,
                  const int &newVaccinatedNode,
                  const std::vector<bool> &Infected_Node,
                  const std::vector<int> & current_infected,
                  const uint64_t &seed,
                  const int &stepSize,
                  const double prob_infect)
{
    // 1) Copy the graph
    Graph::Params params_copy = G.params;
    Graph Gc(params_copy, seed);

    Gc.centers = G.centers;
    Gc.nodes = G.nodes;
    Gc.adj_list = G.adj_list;
    Gc.nodesThreshold = G.nodesThreshold;
    Gc.build_spatial_index();

    // Create a edgeRand struct 
    UndirectedEdgeRandom edgeRand(seed);
    const int N = Gc.nodes.size();

    // Immune mask
    std::vector<int> immune(N, 0);
    for (int i = 0; i < (int)Vaccinated_Node.size() && i < N; i++)
        if (Vaccinated_Node[i]) immune[i] = 1;

    if (newVaccinatedNode != -1)
        immune[newVaccinatedNode] = 1;

    // Initial infection
    std::queue<int> frontier;
    std::vector<bool> infected_mask = Infected_Node;
    int infectedCount = std::count(infected_mask.begin(), infected_mask.end(), true);
    
    for (int u : current_infected) {
        if (!infected_mask[u]) {
            infected_mask[u] = 1;
            infectedCount++;
        }
        frontier.push(u);
    }

    int steps = 0;
    int time = 0 ; 
    // IC propagation
    while (!frontier.empty()) {
        std::queue<int> next_frontier;

        // Movement
        if (steps > 0 && stepSize > 0 && steps % stepSize == 0) {
            Gc.simulate_movement(stepSize);
            time ++ ; 
        }

        while (!frontier.empty()) {
            int u = frontier.front();
            frontier.pop();

            for (int v : Gc.adj_list[u]) {
                if (infected_mask[v] || immune[v]) continue;

                double r = edgeRand.get(u, v, time);

                if (r < prob_infect) {
                    infected_mask[v] = 1;
                    next_frontier.push(v);
                    infectedCount++;
                }
            }
        }

        frontier = next_frontier;
        steps++;
    }

    return N - infectedCount; // return number of saved nodes 
}