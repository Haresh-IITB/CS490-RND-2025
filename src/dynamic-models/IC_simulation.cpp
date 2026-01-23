#include "waxman-graph.h"
#include <queue>
#include <iostream>
#include <iomanip>

int IC_Simulation_dynamic(Graph &G,
                  const std::vector<bool> &Vaccinated_Node,
                  const int &newVaccinatedNode,
                  std::vector<bool> &Infected_Node,
                  std::vector<int> & Out_Newly_Infected,
                  const int & num_steps,
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

    // Immune mask
    std::vector<int> immune(N, 0);
    for (int i = 0; i < (int)Vaccinated_Node.size() && i < N; i++)
        if (Vaccinated_Node[i]) immune[i] = 1;

    if (newVaccinatedNode != -1)
        immune[newVaccinatedNode] = 1;

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

    while (!frontier.empty() && steps < num_steps) {
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
    }

    // We need to pass 2 information the current infected nodes and the total
    while(!frontier.empty()) {
        int u = frontier.front();
        frontier.pop();
        Out_Newly_Infected.push_back(u);
    }
    Infected_Node = infected;

    return N - infectedCount;
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
