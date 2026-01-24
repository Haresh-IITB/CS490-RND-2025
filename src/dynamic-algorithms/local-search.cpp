#include "waxman-graph.h"
#include <iostream>


void Local_Search(
    Graph & G,
    const int & k, 
    const std::vector<int> & initial_infected,
    std::vector<bool> & infected_mask,
    const std::vector<bool> & pre_vaccinated_mask,
    std::vector<int> & out_vaccinated_nodes,
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
    int max_no_improve_iters,
    const double & Prob_infect,
    const std::vector<int> & intial_vaccinated_nodes = std::vector<int>()
) {
    int N = G.nodes.size();
    std::vector<bool> Vaccinated_Node = pre_vaccinated_mask;
    
    std::unordered_set<int> Vaccinated_List;

    for (int u : initial_infected)
        infected_mask[u] = true;

    // Initial random selection
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dis(0, N - 1);

    if(!intial_vaccinated_nodes.empty()) {
        for (int u : intial_vaccinated_nodes) {
            if (!Vaccinated_Node[u] && !infected_mask[u]) {
                Vaccinated_Node[u] = true;
                Vaccinated_List.insert(u);
            }
        }
    }else{
        while ((int)Vaccinated_List.size() < k) {
            int u = dis(gen);
            if (!Vaccinated_Node[u] && !infected_mask[u]) {
                Vaccinated_Node[u] = true;
                Vaccinated_List.insert(u);
            }
        }
    }
    

    // Seeds
    std::vector<uint64_t> array_seed(T);
    for (int t = 0; t < T; ++t)
        array_seed[t] = 2024 + t;

    bool converged = false;
    int no_improve_iters = 0;

    while (!converged && no_improve_iters < max_no_improve_iters) {
        converged = true;

        // SAFE iteration snapshot
        std::vector<int> vaccinated_snapshot(
            Vaccinated_List.begin(), Vaccinated_List.end()
        );

        // Compute baseline ONCE
        int baseline_saved = 0;
        for (int t = 0; t < T; ++t)
            baseline_saved += Simulator(
                G, Vaccinated_Node, -1,
                infected_mask, initial_infected, array_seed[t], stepSize, Prob_infect
            );

        for (int u_out : vaccinated_snapshot) {

            int best_u_in = -1;
            int best_saved = baseline_saved;

            for (int u_in : G.adj_list[u_out]) {
                if (Vaccinated_Node[u_in] || infected_mask[u_in])
                    continue;

                Vaccinated_Node[u_out] = false;
                Vaccinated_Node[u_in] = true;

                int current_saved = 0;
                for (int t = 0; t < T; ++t)
                    current_saved += Simulator(
                        G, Vaccinated_Node, -1,
                        infected_mask, initial_infected, array_seed[t], stepSize, Prob_infect
                    );


                if (current_saved > best_saved) {
                    best_saved = current_saved;
                    best_u_in = u_in;
                }

                Vaccinated_Node[u_out] = true;
                Vaccinated_Node[u_in] = false;
            }

            if (best_u_in != -1) {

                Vaccinated_Node[u_out] = false;
                Vaccinated_Node[best_u_in] = true;
                Vaccinated_List.erase(u_out);
                Vaccinated_List.insert(best_u_in);

                converged = false;
                break;
            }
        }

        no_improve_iters++;
    }

    out_vaccinated_nodes.assign(
        Vaccinated_List.begin(),
        Vaccinated_List.end()
    );

}
