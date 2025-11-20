// IndependentCascade.cpp
// dynamic Independent Cascade on a COPY of the Graph:
// at each timestep:
//   1) infected frontier attempts to infect neighbors (one-shot per neighbor)
//   2) graph moves nodes (simulate_movement(1)) and rewires edges
// repeats until no new infections

#include "waxman-graph.h"
#include "Random_number_generator.h"
#include <vector>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <iostream>

// Infection probability per attempted edge 
const double INFECTION_P = 0.05;

// IC_Simulation signature matches your earlier usage:
// returns number of SAVED nodes (V - infectedCount)
int IC_Simulation(Graph &G,
                  std::vector<bool> &isVaccinable,           // original vaccinable flags (NOT modified)
                  const int &newVaccinatedNode,              // -1 if none, otherwise node index to temporarily vaccinate
                  const std::vector<int> &infected_nodes)   // initial infected set (indices)
{

    // Construct new graph with same Params
    Graph::Params params_copy = G.params;
    Graph Gc(params_copy, 1337);

    // copy structural data
    Gc.centers = G.centers;
    Gc.nodes = G.nodes;
    Gc.adj_list = G.adj_list;

    // rebuild spatial index for the copied nodes 
    Gc.build_spatial_index();

    // --- Prepare vaccination / infectable maps (do NOT change original isVaccinable) ---
    const int N = (int)Gc.nodes.size();
    std::vector<int> isInfectable(N, 1); // int for compactness

    // start by marking non-infectable all nodes that are not vaccinable (i.e., already vaccinated)
    for (int i = 0; i < isVaccinable.size() && i < N; ++i) {
        if (!isVaccinable[i]) isInfectable[i] = 0;
    }

    // apply the candidate vaccination (temporary)
    if (newVaccinatedNode != -1 && newVaccinatedNode >= 0 && newVaccinatedNode < N) {
        isInfectable[newVaccinatedNode] = 0;
    }

    // --- Initialize RNG for infection draws (independent from Graph internals) ---
    Random_number_generator rng(1337);

    // --- Initialize infection state ---
    std::vector<int> infected(N, 0); // who has ever been infected
    std::queue<int> frontier;         // nodes that will attempt infection this timestep

    // Seed initial infected nodes (but skip those that are vaccinated)
    int infectedCount = 0;
    for (int u : infected_nodes) {
        if (u < 0 || u >= N) continue;
        if (!isInfectable[u]) continue; // vaccinated -> cannot be infected
        if (!infected[u]) {
            infected[u] = 1;
            frontier.push(u);
            infectedCount++;
        }
    }

    // std::cout << "Initial infected count in IC_Simulation: " << infectedCount << "\n";

    // If nothing is initially infected (or all vaccinated), return saved = N - 0 = N
    if (frontier.empty()) {
        return N - infectedCount;
    }


    // Each iteration: current frontier attempts infections; then we move nodes/rewire; new infections form next frontier
    while (!frontier.empty()) {
        std::queue<int> next_frontier;

        // Process each active infector u
        while (!frontier.empty()) {
            int u = frontier.front();
            frontier.pop();

            // iterate over neighbors in the copied graph's adjacency list
            // note: adj_list[u] contains neighbor ids
            for (int v : Gc.adj_list[u]) {
                if (v < 0 || v >= N) continue;
                if (!isInfectable[v]) continue;   // vaccinated / not infectable
                if (infected[v]) continue;       // already infected earlier

                // attempt infection with probability INFECTION_P
                int succ = rng.get_bernoulli(INFECTION_P);
                if (succ) {
                    infected[v] = 1;
                    next_frontier.push(v);
                    infectedCount++;
                }
            }
        } 

        // After infection attempts at this timestep, perform movement + rewiring
        // This models edge changes between timesteps (dynamic graph) ** TUNABLE Parameter** , so you can adjust movement frequency
        if (next_frontier.empty()) {
            // no new infections; still could move nodes but since no future infections will occur, we can break early
            break;
        } else {
            // Move nodes by one step (this updates positions and adjacency via your Graph implementation)
            Gc.simulate_movement(1);

            // Now use next_frontier as frontier for next iteration
            frontier = std::move(next_frontier);
        }
    } // while frontier

    // Compute saved nodes (those not infected)
    int saved = N - infectedCount;
    return saved;
}
