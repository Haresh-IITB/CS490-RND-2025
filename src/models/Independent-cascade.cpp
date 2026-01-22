#include "waxman-graph.h"
#include <queue>
#include <iostream>
#include <iomanip>

#define DEBUG_IC 0

int IC_Simulation(Graph &G,
                  const std::vector<bool> &Vaccinated_Node,
                  const int &newVaccinatedNode,
                  const std::vector<bool> &Infected_Node,
                  const uint64_t &seed,
                  const int &stepSize,
                  const double prob_infect)
{
#if DEBUG_IC
    std::cout << "\n================ IC SIMULATION START ================\n";
    std::cout << "Seed = " << seed
              << ", newVaccinatedNode = " << newVaccinatedNode
              << ", stepSize = " << stepSize << "\n";
#endif

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

#if DEBUG_IC
    std::cout << "Immune nodes: ";
    for (int i = 0; i < N; ++i)
        if (immune[i]) std::cout << i << " ";
    std::cout << "\n";
#endif

    // Initial infection
    std::vector<int> infected(N, 0);
    std::queue<int> frontier;
    int infectedCount = 0;

#if DEBUG_IC
    std::cout << "Initial infected: ";
#endif
    for (int u = 0; u < N; ++u) {
        if (Infected_Node[u]) {
            infected[u] = 1;
            frontier.push(u);
            infectedCount++;
#if DEBUG_IC
            std::cout << u << " ";
#endif
        }
    }
#if DEBUG_IC
    std::cout << "\n";
#endif

    int steps = 0;
    int time = 0 ; 
    // IC propagation
    while (!frontier.empty()) {

#if DEBUG_IC
        std::cout << "\n--- IC STEP " << steps << " ---\n";
        std::cout << "Frontier: ";
        std::queue<int> tmp = frontier;
        while (!tmp.empty()) {
            std::cout << tmp.front() << " ";
            tmp.pop();
        }
        std::cout << "\n";
#endif

        std::queue<int> next_frontier;

        // Movement
        if (steps > 0 && stepSize > 0 && steps % stepSize == 0) {
            Gc.simulate_movement(stepSize);
            time ++ ; 
            #if DEBUG_IC
                        std::cout << ">>> Movement triggered ("
                                  << stepSize << " steps)\n";
                        // Adjanecy list after movement
                        std::cout << "Adjacency List after movement:\n";
                        for (int i = 0; i < N; ++i) {
                            std::cout << "Node " << i << ": ";
                            for (int v : Gc.adj_list[i])
                                std::cout << v << " ";
                            std::cout << "\n";
                        }
            #endif
        }

        while (!frontier.empty()) {
            int u = frontier.front();
            frontier.pop();

            for (int v : Gc.adj_list[u]) {
                if (infected[v] || immune[v]) continue;

                double r = edgeRand.get(u, v, time);

#if DEBUG_IC
                std::cout << "Try infect: " << u << " -> " << v
                          << " | d=" << std::fixed << std::setprecision(3) << d
                          << " p=" << p
                          << " r=" << r;
#endif

                if (r < prob_infect) {
                    infected[v] = 1;
                    next_frontier.push(v);
                    infectedCount++;
#if DEBUG_IC
                    std::cout << "  [INFECTED]\n";
#endif
                } else {
#if DEBUG_IC
                    std::cout << "  [FAIL]\n";
#endif
                }
            }
        }

        frontier = next_frontier;
        steps++;
    }

#if DEBUG_IC
    std::cout << "\nFinal infected nodes: ";
    for (int i = 0; i < N; ++i)
        if (infected[i]) std::cout << i << " ";
    std::cout << "\n";

    std::cout << "Total infected = " << infectedCount
              << ", saved = " << (N - infectedCount) << "\n";
    std::cout << "================ IC SIMULATION END =================\n";
#endif
    return N - infectedCount; // return number of saved nodes 
}
