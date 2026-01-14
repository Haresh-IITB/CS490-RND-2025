#include "waxman-graph.h"
#include <iostream>

// #define DEBUG 

#ifdef DEBUG
    #define DBG(x) do { std::cerr << x << std::endl; } while(0)
#else
    #define DBG(x) do {} while(0)
#endif


void Local_Search(
    Graph & G,
    const int & k, 
    const std::vector<int> & initial_infected,
    std::vector<int> & out_vaccinated_nodes,
    std::function<int(
        Graph &,
        const std::vector<bool> &,
        const int &,
        const std::vector<bool> &,
        const uint64_t &,
        const int &
    )> Simulator,
    int stepSize,
    int T,
    int max_no_improve_iters,
    const std::vector<int> & intial_vaccinated_nodes = std::vector<int>()
) {
    int N = G.nodes.size();
    std::vector<bool> Vaccinated_Node(N, false);
    std::vector<bool> Infected_Node(N, false);
    std::unordered_set<int> Vaccinated_List;

    for (int u : initial_infected)
        Infected_Node[u] = true;

    // Initial random selection
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dis(0, N - 1);

    if(!intial_vaccinated_nodes.empty()) {
        for (int u : intial_vaccinated_nodes) {
            if (!Vaccinated_Node[u] && !Infected_Node[u]) {
                Vaccinated_Node[u] = true;
                Vaccinated_List.insert(u);
            }
        }
    }else{
        while ((int)Vaccinated_List.size() < k) {
            int u = dis(gen);
            if (!Vaccinated_Node[u] && !Infected_Node[u]) {
                Vaccinated_Node[u] = true;
                Vaccinated_List.insert(u);
            }
        }
    }
    
    DBG("[INIT] Initial vaccinated size = " << Vaccinated_List.size());

    // Seeds
    std::vector<uint64_t> array_seed(T);
    for (int t = 0; t < T; ++t)
        array_seed[t] = 2024 + t;

    bool converged = false;
    int no_improve_iters = 0;

    while (!converged && no_improve_iters < max_no_improve_iters) {
        converged = true;
        DBG("\n[ITER] no_improve_iters = " << no_improve_iters);

        // SAFE iteration snapshot
        std::vector<int> vaccinated_snapshot(
            Vaccinated_List.begin(), Vaccinated_List.end()
        );

        // Compute baseline ONCE
        int baseline_saved = 0;
        for (int t = 0; t < T; ++t)
            baseline_saved += Simulator(
                G, Vaccinated_Node, -1,
                Infected_Node, array_seed[t], stepSize
            );

        DBG("  Baseline saved = " << baseline_saved);

        for (int u_out : vaccinated_snapshot) {

            int best_u_in = -1;
            int best_saved = baseline_saved;

            for (int u_in : G.adj_list[u_out]) {
                if (Vaccinated_Node[u_in] || Infected_Node[u_in])
                    continue;

                Vaccinated_Node[u_out] = false;
                Vaccinated_Node[u_in] = true;

                int current_saved = 0;
                for (int t = 0; t < T; ++t)
                    current_saved += Simulator(
                        G, Vaccinated_Node, -1,
                        Infected_Node, array_seed[t], stepSize
                    );

                DBG("    Try swap OUT=" << u_out
                    << " IN=" << u_in
                    << " saved=" << current_saved);

                if (current_saved > best_saved) {
                    best_saved = current_saved;
                    best_u_in = u_in;
                }

                Vaccinated_Node[u_out] = true;
                Vaccinated_Node[u_in] = false;
            }

            if (best_u_in != -1) {
                DBG("  ACCEPT swap: OUT=" << u_out
                    << " IN=" << best_u_in
                    << " gain=" << (best_saved - baseline_saved));

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

    DBG("[DONE] Final vaccinated size = " << out_vaccinated_nodes.size());
}
