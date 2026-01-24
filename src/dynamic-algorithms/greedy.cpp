#include "waxman-graph.h"
#include <iostream>
#include <iomanip>

void Greedy_Vaccination_Dynamic(
    Graph &G, 
    int k, 
    const std::vector<int> &current_infected,
    std::vector<bool> &Infected_Node,
    const std::vector<bool> &pre_vaccinated_mask,
    std::vector<int> &out_new_vaccines,
    std::function<int(
                            Graph &,
                            const std::vector<bool> &,
                            const int &,
                            const std::vector<bool> &,
                            const std::vector<int> &,
                            const uint64_t &,
                            const int &,
                            const double 
                        )> Simulator,
    int stepSize,
    int T,
    const double & Prob_infect
) {
    int N = G.nodes.size();
    std::vector<bool> Vaccinated_Node = pre_vaccinated_mask;
    
    for (int u : current_infected) Infected_Node[u] = true;

    out_new_vaccines.clear();

    std::vector<uint64_t> sim_seeds(T);
    for(int i=0; i<T; ++i) sim_seeds[i] = 1000 + i;

    std::cout << "[Greedy] Selecting " << k << " nodes...\n";

    for (int iter = 0; iter < k; ++iter) {
        int best_node = -1;
        double max_marginal_gain = -1.0;

        for (int u = 0; u < N; ++u) {
            if (Vaccinated_Node[u] || Infected_Node[u]) continue;

            long long current_saved = 0;
            for (int t = 0; t < T; ++t) {
                current_saved += Simulator(G, Vaccinated_Node, u, Infected_Node, current_infected, sim_seeds[t], stepSize, Prob_infect);
            }

            double gain = (double)(current_saved);
            
            if (gain > max_marginal_gain) {
                max_marginal_gain = gain;
                best_node = u;
            }
        }

        if (best_node != -1) {
            Vaccinated_Node[best_node] = true;
            out_new_vaccines.push_back(best_node);
        } else {
            break; 
        }
    }
}