// LinearThreshold.cpp
// Dynamic Linear Threshold model on a COPY of the Graph
// Each timestep:
//   1) infected frontier contributes influence to neighbors
//   2) nodes crossing threshold activate
//   3) graph moves (simulate_movement(1)) + rewires edges
// Repeat until no new activations.

// Assumed that influence weight from u to v is 1/deg(v)

// The waxmaan weights are very small try to multiply it by a factor **** 

// #define DEBUG 

#include "waxman-graph.h"
#include <queue>
#include <iostream>

#ifdef DEBUG
#define DBG(x) do { std::cerr << x << std::endl; } while(0)
#else
#define DBG(x) do {} while(0)
#endif


int LinearThreshold_Simulator(
    Graph & G,
    const std::vector<bool> & Vaccinated_Node,
    const int & newVaccinatedNode, 
    const std::vector<bool> & Infected_Node,
    const uint64_t & seed,
    const int & stepSize
) {
    DBG("[LT] Simulator start | seed=" << seed
        << " stepSize=" << stepSize);

    // 1) Copy the graph
    Graph::Params params_copy = G.params;
    Graph Gc(params_copy, seed);
    Gc.centers = G.centers;
    Gc.nodes = G.nodes;
    Gc.adj_list = G.adj_list;
    Gc.nodesThreshold = G.nodesThreshold;
    Gc.build_spatial_index();
    
    const int N = G.nodes.size();

    std::vector<bool> Active_Node(N, false);
    std::vector<double> Accumulated_Influence(N, 0.0);

    // Immune mask
    std::vector<int> immune(N, 0);
    for (int i = 0; i < (int)Vaccinated_Node.size() && i < N; i++)
        if (Vaccinated_Node[i]) immune[i] = 1;

    if (newVaccinatedNode != -1)
        immune[newVaccinatedNode] = 1;

    // Initialize active nodes
    std::queue<int> frontier;
    for (int u = 0; u < N; ++u) {
        if (Infected_Node[u] && !immune[u]) {
            Active_Node[u] = true;
            frontier.push(u);
        }
    }

    int activeCount = frontier.size();
    int steps = 0;
    int time  = 0;   

    DBG("[LT] Initial active nodes=" << activeCount);

    while (!frontier.empty()) {

        DBG("[LT] Step " << steps
            << " | frontier size=" << frontier.size());

        std::queue<int> next_frontier;

        // 🔹 Dynamic movement (MATCHES IC)
        if (steps > 0 && stepSize > 0 && steps % stepSize == 0) {
            DBG("[LT] Movement triggered at step " << steps);
            Gc.simulate_movement(stepSize);
            time++;   // 🔹 advance logical time
        }

        while (!frontier.empty()) {
            int u = frontier.front();
            frontier.pop();

            for (int v : Gc.adj_list[u]) {
                if (Active_Node[v] || immune[v]) continue;

                // Influence weight
                double weight =
                    3.0 / std::max(1, (int)Gc.adj_list[v].size());

                Accumulated_Influence[v] += weight;

                DBG("[LT] u=" << u
                    << " -> v=" << v
                    << " | influence=" << Accumulated_Influence[v]
                    << " threshold=" << Gc.nodesThreshold[v]);

                if (Accumulated_Influence[v] >= Gc.nodesThreshold[v]) {
                    Active_Node[v] = true;
                    next_frontier.push(v);
                    activeCount++;

                    DBG("[LT] Node " << v
                        << " ACTIVATED at step "
                        << steps << " time=" << time);
                }
            }
        }

        frontier = next_frontier;
        steps++;
    }

    DBG("[LT] Simulation end | active="
        << activeCount
        << " saved=" << (N - activeCount)
        << " steps=" << steps
        << " time=" << time);

    return N - activeCount;
}
