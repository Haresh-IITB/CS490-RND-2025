// LinearThreshold.cpp
// Dynamic Linear Threshold model on a COPY of the Graph
// Each timestep:
//   1) infected frontier contributes influence to neighbors
//   2) nodes crossing threshold activate
//   3) graph moves (simulate_movement(1)) + rewires edges
// Repeat until no new activations.

#include "waxman-graph.h"
#include "Random_number_generator.h"
#include <vector>
#include <queue>
#include <cmath>

// LT_Simulation:
// returns number of saved nodes = V - infected_count
int LT_Simulation(Graph &G,
                  std::vector<bool> &isVaccinable,           // original vaccination masks (NOT modified)
                  const int &newVaccinatedNode,              // vaccinated candidate
                  const std::vector<int> &infected_nodes)   // initial infected
{
    // 1) Copy the graph (so evaluator does NOT mutate original)
    Graph::Params params_copy = G.params;
    Graph Gc(params_copy, 1337);   // deterministic seed for fairness

    Gc.centers = G.centers;
    Gc.nodes   = G.nodes;
    Gc.adj_list = G.adj_list;
    Gc.build_spatial_index();

    const int N = Gc.nodes.size();


    // 2) Build infectability mask
    std::vector<char> infectable(N, 1);   // 1 = can be infected
    for (int i = 0; i < (int)isVaccinable.size() && i < N; i++) {
        if (!isVaccinable[i])
            infectable[i] = 0;
    }
    if (newVaccinatedNode != -1)
        infectable[newVaccinatedNode] = 0;

    // 3) Initialize LT influence scores + infection state
    std::vector<double> influence(N, 0.0);
    std::vector<char> infected(N, 0);

    std::queue<int> frontier;

    int infectedCount = 0;
    for (int u : infected_nodes) {
        if (u < 0 || u >= N) continue;
        if (!infectable[u]) continue;
        infected[u] = 1;
        frontier.push(u);
        infectedCount++;
    }

    if (frontier.empty()) {
        return N - infectedCount;  // nothing to infect
    }

    // 4) LT Spread + movement loop
    while (!frontier.empty()) {
        std::queue<int> next_frontier;

        // Phase A: influence propagation 
        while (!frontier.empty()) {
            int u = frontier.front();
            frontier.pop();

            // iterate neighbors of u in copied graph
            for (int v : Gc.adj_list[u]) {
                if (!infectable[v]) continue;   // vaccinated
                if (infected[v]) continue;      // already active

                // weight = Waxman probability p(u,v)
                double w = G.params.alpha * std::exp(
                    -Gc.distance(Gc.nodes[u].x, Gc.nodes[u].y, 
                                 Gc.nodes[v].x, Gc.nodes[v].y) / 
                      G.params.beta
                );

                influence[v] += w;

                // check threshold crossing
                if (influence[v] >= Gc.nodesThreshold[v]) {
                    infected[v] = 1;
                    next_frontier.push(v);
                    infectedCount++;
                }
            }
        }

        // No new activations → stop here 
        if (next_frontier.empty()) break;

        // Phase B: move graph + rewire edges 
        Gc.simulate_movement(1);

        frontier = std::move(next_frontier);
    }

    return N - infectedCount; // number of saved nodes
}
