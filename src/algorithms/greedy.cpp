#include "waxman-graph.h"
#include <iostream>
#include <iomanip>

#define DEBUG_GREEDY 0

void Greedy_Vaccination(Graph &G, 
                        const int &k, 
                        const std::vector<int> &initial_infected,
                        std::vector<int> &out_vaccinated_nodes,
                        std::function<int(
                            Graph &,
                            const std::vector<bool> &,
                            const int &,
                            const std::vector<bool> &,
                            const uint64_t &,
                            const int &
                        )> Simulator,
                        int stepSize,
                        int T) 
{
    int N = G.nodes.size();
    std::vector<bool> Vaccinated_Node(N, false);
    std::vector<bool> Infected_Node(N, false);

    for (int u : initial_infected)
        Infected_Node[u] = true;

#if DEBUG_GREEDY
    std::cout << "\n===== GREEDY VACCINATION START =====\n";
    std::cout << "Total nodes: " << N
              << ", k = " << k
              << ", T = " << T
              << ", stepSize = " << stepSize << "\n";
#endif

    for (int iter = 0; iter < k; ++iter) {

#if DEBUG_GREEDY
        std::cout << "\n--- Iteration " << iter + 1 << " ---\n";
        std::cout << "Current vaccinated set: ";
        for (int i = 0; i < N; ++i)
            if (Vaccinated_Node[i]) std::cout << i << " ";
        std::cout << "\n";
#endif

        std::vector<int> saved(N, 0);

        // Seeds
        std::vector<uint64_t> array_seed;
        for (int t = 0; t < T; ++t) {
            array_seed.push_back(1337 + iter * T + t);
        }

#if DEBUG_GREEDY
        std::cout << "Seeds: ";
        for (auto s : array_seed) std::cout << s << " ";
        std::cout << "\n";
#endif

        // Baseline saved
        std::vector<int> currSaved(T, 0);
        for (int t = 0; t < T; ++t) {
            currSaved[t] = Simulator(
                G, Vaccinated_Node, -1,
                Infected_Node, array_seed[t], stepSize
            );
        }

#if DEBUG_GREEDY
        std::cout << "Baseline saved (no new vaccine): ";
        for (int t = 0; t < T; ++t)
            std::cout << currSaved[t] << " ";
        std::cout << "\n";
#endif

        // Evaluate candidates
        for (int u = 0; u < N; ++u) {
            if (Vaccinated_Node[u] || Infected_Node[u]) continue;

#if DEBUG_GREEDY
            std::cout << "Candidate u = " << u << " : ";
#endif

            int total_gain = 0;
            for (int t = 0; t < T; ++t) {
                int with_u = Simulator(
                    G, Vaccinated_Node, u,
                    Infected_Node, array_seed[t], stepSize
                );
                int gain = with_u - currSaved[t];
                total_gain += gain;

#if DEBUG_GREEDY
                std::cout << "[t" << t
                          << ": +" << gain << "] ";
#endif
            }

            saved[u] = total_gain;

#if DEBUG_GREEDY
            std::cout << " => total gain = " << total_gain << "\n";
#endif
        }

        // Select best u
        int u_star = -1;
        int max_saved = -1;

        for (int u = 0; u < N; ++u) {
            if (Vaccinated_Node[u] || Infected_Node[u]) continue;
            if (saved[u] > max_saved) {
                max_saved = saved[u];
                u_star = u;
            }
        }

        if (u_star == -1) {
#if DEBUG_GREEDY
            std::cout << "No valid candidate found. Stopping.\n";
#endif
            break;
        }

        Vaccinated_Node[u_star] = true;
        out_vaccinated_nodes.push_back(u_star);

#if DEBUG_GREEDY
        std::cout << ">>> SELECTED u* = " << u_star
                  << " with marginal gain = " << max_saved << "\n";
#endif
    }

#if DEBUG_GREEDY
    std::cout << "\n===== GREEDY VACCINATION END =====\n";
#endif
}