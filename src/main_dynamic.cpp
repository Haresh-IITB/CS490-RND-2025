#include "waxman-graph.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <functional>
#include <config.h>
#include <set>

int IC_Simulation(Graph &G,
                  const std::vector<bool> &Vaccinated_Node,
                  const int &newVaccinatedNode,
                  const std::vector<bool> &Infected_Node,
                  const uint64_t &seed,
                  const int &stepSize);

void Greedy_Vaccination_Dynamic(
    Graph &G, 
    int k, 
    const std::vector<int> &current_infected,
    const std::vector<bool> &pre_vaccinated_mask,
    std::vector<int> &out_new_vaccines,
    int stepSize,
    int T 
) {
    int N = G.nodes.size();
    std::vector<bool> Vaccinated_Node = pre_vaccinated_mask;
    
    std::vector<bool> Infected_Node(N, false);
    for (int u : current_infected) Infected_Node[u] = true;

    out_new_vaccines.clear();

    std::vector<uint64_t> sim_seeds(T);
    for(int i=0; i<T; ++i) sim_seeds[i] = 1000 + i;

    std::cout << "[Greedy] Selecting " << k << " nodes...\n";

    for (int iter = 0; iter < k; ++iter) {
        int best_node = -1;
        double max_marginal_gain = -1.0;
        
        long long baseline_saved = 0;
        for (int t = 0; t < T; ++t) {
            baseline_saved += IC_Simulation(G, Vaccinated_Node, -1, Infected_Node, sim_seeds[t], stepSize);
        }
        for (int u = 0; u < N; ++u) {
            if (Vaccinated_Node[u] || Infected_Node[u]) continue;

            long long current_saved = 0;
            for (int t = 0; t < T; ++t) {
                current_saved += IC_Simulation(G, Vaccinated_Node, u, Infected_Node, sim_seeds[t], stepSize);
            }

            double gain = (double)(current_saved - baseline_saved);
            
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


void Run_Real_World_Transition(
    Graph &G,
    std::vector<bool> &Infected_Node,
    const std::vector<bool> &Vaccinated_Node,
    int steps_duration,
    uint64_t seed
) {
    UndirectedEdgeRandom edgeRand(seed); 
    int N = G.nodes.size();

    double weight = 5.0; // Same as in Independent-cascade.cpp

    for (int t = 0; t < steps_duration; ++t) {
        G.simulate_movement(1); 

        std::vector<int> frontier;
        for(int i=0; i<N; ++i) if(Infected_Node[i]) frontier.push_back(i);

        std::vector<int> newly_infected;

        for (int u : frontier) {
            for (int v : G.adj_list[u]) {
                if (Infected_Node[v] || Vaccinated_Node[v]) continue;

                double d = G.distance(G.nodes[u].x, G.nodes[u].y, G.nodes[v].x, G.nodes[v].y);
                double p = std::min(1.0, weight * G.params.alpha * std::exp(-d / G.params.beta));
                
                // Roll probability
                if (edgeRand.get(u, v, t + seed) < p) {
                   newly_infected.push_back(v);
                }
            }
        }

        // Apply infections
        for(int v : newly_infected) Infected_Node[v] = true;        
    }
}

int Run_Dynamic_Vaccination(
    Graph &G,
    const std::vector<int> &batch_sizes, 
    int steps_between_batches,          
    const std::vector<int> &initial_infected
) {
    int N = G.nodes.size();
    std::vector<bool> Global_Vaccinated(N, false);
    std::vector<bool> Global_Infected(N, false);
    
    for(int u : initial_infected) Global_Infected[u] = true;

    std::cout << "\n=== STARTING DYNAMIC VACCINATION ===\n";
    std::cout << "Total Nodes: " << N << "\n";
    std::cout << "Initial Infected: " << initial_infected.size() << "\n";

    for (size_t b = 0; b < batch_sizes.size(); ++b) {
        int k = batch_sizes[b];
        std::cout << "\n--- Batch " << b+1 << " (Budget: " << k << ") ---\n";

        std::vector<int> curr_infected_list;
        int inf_count = 0;
        for(int i=0; i<N; ++i) if(Global_Infected[i]) { curr_infected_list.push_back(i); inf_count++; }

        std::cout << "Current Status -> Infected: " << inf_count << ", Vaccinated: " << std::count(Global_Vaccinated.begin(), Global_Vaccinated.end(), true) << "\n";

        std::vector<int> new_vaccines;        
        Greedy_Vaccination_Dynamic(G, k, curr_infected_list, Global_Vaccinated, new_vaccines, steps_between_batches, 10);

        // 3. Apply Vaccines
        std::cout << "Allocated Vaccines to nodes: ";
        for(int v : new_vaccines) {
            Global_Vaccinated[v] = true;
            std::cout << v << " ";
        }
        std::cout << "\n";

        // 4. Transition (Real World Simulation)
        std::cout << "Simulating " << steps_between_batches << " steps of real-world spread...\n";
        Run_Real_World_Transition(G, Global_Infected, Global_Vaccinated, steps_between_batches, 9999 + b);
    }

    int final_infected = 0;
    for(bool x : Global_Infected) if(x) final_infected++;
    
    std::cout << "\n=== SIMULATION COMPLETE ===\n";
    std::cout << "Final Infected Count: " << final_infected << "\n";
    std::cout << "Total Vaccinated: " << std::count(Global_Vaccinated.begin(), Global_Vaccinated.end(), true) << "\n";
    std::cout << "Healthy (Saved): " << N - final_infected << "\n";

    return N - final_infected; // The number of nodes saved ; 
}

std::vector<int> sample_initial_infected(
    const Graph &G,
    double frac,
    uint64_t seed) {

    int N = G.nodes.size();
    int k = static_cast<int>(frac * N);

    std::set<int> s;
    Random_number_generator rng(seed);

    while ((int)s.size() < k) {
        s.insert(rng.get_int(0, N - 1));
    }
    return std::vector<int>(s.begin(), s.end());
}

int main() {
    std::cout << "Program started: Dynamic Greedy Vaccination (Default Config)\n";

    Config cfg;
    cfg.node_sizes = {256};             // N = 256
    cfg.seed = 2025;
    cfg.num_centers = 5;
    cfg.alpha = 0.05;                     // Waxman alpha
    cfg.beta = 0.2;                      // Waxman beta
    cfg.cutoff_prob = 1e-3;
    cfg.initial_infected_percent = 0.05; // 5% infected
    cfg.vaccination_budget_percent = 0.10;// 10% budget
    cfg.diffusion_model = "IC";

    int N = cfg.node_sizes.front();
    std::cout << "Config set. N=" << N << ", Seed=" << cfg.seed << "\n";

    Graph::Params p;
    p.num_cities = cfg.num_centers;
    p.num_villages = cfg.num_centers * 2;
    p.nodes_per_city = static_cast<int>(0.7 * N / cfg.num_centers);
    p.nodes_per_village = (N - p.nodes_per_city * cfg.num_centers) / p.num_villages;
    p.alpha = cfg.alpha;
    p.beta = cfg.beta;
    p.cutoff_prob = cfg.cutoff_prob;
    if(p.nodes_per_village <= 0) p.nodes_per_village = 1;

    std::cout << "Generating Graph...\n";
    Graph G(p, cfg.seed);
    G.generate_centers();
    G.generate_nodes();
    G.build_spatial_index();
    G.generate_edges();
    std::cout << "Graph generated. Nodes: " << G.nodes.size() << " Edges: " << G.adj_list.size() << "\n";

    std::vector<int> initial_infected = sample_initial_infected(G, cfg.initial_infected_percent, cfg.seed);
    
    int total_budget = static_cast<int>(cfg.vaccination_budget_percent * N);
    
    int batches = 3;
    int batch_k = total_budget / batches;
    int remainder = total_budget % batches;

    std::vector<int> schedule;
    for(int i=0; i<batches; ++i) {
        if (i == batches - 1) schedule.push_back(batch_k + remainder);
        else schedule.push_back(batch_k);
    }
    
    int time_interval = 3; 

    int result_dynamic = Run_Dynamic_Vaccination(G, schedule, time_interval, initial_infected);

    // The result of running the normal greedy vaccination can be compared by calling:
    std::vector<int> normal_vaccines;
    Greedy_Vaccination_Dynamic(G, total_budget, initial_infected, std::vector<bool>(G.nodes.size(), false) ,  normal_vaccines,  3, 10);
    // Apply normal vaccination
    std::vector<bool> normal_vaccinated_mask(N, false);
    for(int v : normal_vaccines) normal_vaccinated_mask[v] = true;
    std::vector<bool> normal_infected_mask(N, false);
    for(int u : initial_infected) normal_infected_mask[u] = true;
    int result_normal = 0;
    {
        int saved_count = 0;
        std::vector<uint64_t> sim_seeds(10);
        for(int i=0; i<10; ++i) sim_seeds[i] = 2000 + i;
        for (int t = 0; t < 10; ++t) {
            int prev_saved_cnt = saved_count; 
            saved_count += IC_Simulation(G, normal_vaccinated_mask, -1, normal_infected_mask, sim_seeds[t], 3);
            std::cout << saved_count - prev_saved_cnt << " ";
        }
        result_normal = saved_count / 10;
    }

    std::cout << "\n=== SUMMARY ===\n";
    std::cout << "Dynamic Vaccination Saved: " << result_dynamic << "\n";
    std::cout << "Normal Vaccination Saved: " << result_normal << "\n";

    return 0;
}