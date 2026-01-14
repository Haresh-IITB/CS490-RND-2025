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
    
    // Generate the thershold based on the seed 
    for(int i = 0; i < (int)Gc.nodes.size(); i++) {
        Gc.nodesThreshold[i] = Gc.rng_gen->get_unif();
    }

    const int N = G.nodes.size();
    std::vector<bool> Active_Node(N, false);
    std::vector<double> Accumulated_Influence(N, 0.0);

    // Immune Mask 
    std::vector<int> immune(N, 0);
    for (int i = 0; i < (int)Vaccinated_Node.size() && i < N; i++)
        if (Vaccinated_Node[i]) immune[i] = 1;
    
    if(newVaccinatedNode != -1)
        immune[newVaccinatedNode] = 1;

    int vaccinatedCount = 0;
    for (int i = 0; i < N; ++i)
        if (immune[i]) vaccinatedCount++;

    DBG("[LT] Total nodes=" << N
        << " vaccinated=" << vaccinatedCount);

    // Initialize active nodes
    for (int u = 0; u < N; ++u) {
        if (Infected_Node[u] && !Vaccinated_Node[u]) {
            Active_Node[u] = true;
        }
    }

    // Use a BFS approach for influence propagation
    std::queue<int> frontier;
    for (int u = 0; u < N; ++u) {
        if (Active_Node[u]) {
            frontier.push(u);
        }
    }

    int activeCount = frontier.size();
    int steps = 0;

    DBG("[LT] Initial active nodes=" << activeCount);

    while(!frontier.empty()) {
        std::queue<int> next_frontier;

        DBG("[LT] Step " << steps
            << " | frontier size=" << frontier.size());

        if(steps > 0 && stepSize > 0 && steps % stepSize == 0) {
            DBG("[LT] Simulating movement at step " << steps);
            Gc.simulate_movement(stepSize);
        }

        while(!frontier.empty()) {
            int u = frontier.front();
            frontier.pop();

            for (int v : Gc.adj_list[u]) {
                if (Active_Node[v] || immune[v]) continue;

                // Compute influence weight from u to v
                double weight = 1.0 / Gc.adj_list[v].size();
                Accumulated_Influence[v] += weight;

                DBG("[LT] u=" << u
                    << " -> v=" << v
                    << " | influence=" << Accumulated_Influence[v]
                    << " threshold=" << Gc.nodesThreshold[v]);

                if(Accumulated_Influence[v] >= Gc.nodesThreshold[v]) {
                    Active_Node[v] = true;
                    next_frontier.push(v);
                    activeCount++;

                    DBG("[LT] Node " << v
                        << " ACTIVATED at step " << steps);
                }
            }
        }

        steps++;
        frontier = next_frontier;
    }

    DBG("[LT] Simulation end | total active="
        << activeCount
        << " inactive=" << (N - activeCount)
        << " steps=" << steps);

    return N - activeCount;
}
