#include "waxman-graph.h"
#include "strategy.h" // Assuming this holds your Graph/RoundingMethod definitions
#include "gurobi_c++.h"
#include "sampler.h"

#include <vector>
#include <unordered_set>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>
#include <random>
#include <set>


static void simulate_infection_spread(
    const Graph &G, 
    std::vector<int> &infected_list, // Input/Output
    const std::vector<bool> &vaccinated_mask, 
    int steps, 
    uint64_t seed,
    double prob_infect
) {
    const double P = prob_infect ; // Infection probability factor
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uni(0.0, 1.0);
    
    std::unordered_set<int> newly_infected(infected_list.begin(), infected_list.end());
    std::vector<int> frontier = infected_list;

    for (int t = 0; t < steps; ++t) {
        std::vector<int> next_frontier;
        for (int u : frontier) {
            for (int v : G.adj_list[u]) {
                // If v is susceptible (not infected, not vaccinated)
                if (newly_infected.find(v) == newly_infected.end() && !vaccinated_mask[v]) {
                    // Check transmission prob
                    if (P > uni(rng)) {
                        newly_infected.insert(v);
                        next_frontier.push_back(v);
                    }
                }
            }
        }
        frontier = next_frontier;
        if (frontier.empty()) break;
    }

    // Update the master list
    infected_list.assign(newly_infected.begin(), newly_infected.end()); // Contains all the infected nodes after simulation
}

static GRBEnv GLOBAL_ENV;

struct LPModel {
    GRBModel model;
    int N, S;
    std::vector<GRBVar> I; // Vaccination decision
    std::vector<std::vector<GRBVar>> x; // State (Infected/Safe)

    LPModel(int N_, int S_) : model(GLOBAL_ENV), N(N_), S(S_) {
        model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
        model.set(GRB_IntParam_OutputFlag, 0); // Silence output
        
        I.resize(N);
        x.assign(N, std::vector<GRBVar>(S));
    }
};

static LPModel build_lp_model(
    const std::vector<AdjList> &samples,
    const std::vector<int> &current_infected,
    const std::vector<bool> &pre_vaccinated_mask,
    const std::vector<bool> &already_infected_mask,
    int total_cumulative_budget
) {
    const int S = samples.size();
    const int N = samples[0].size();

    LPModel lp(N, S);

    // --- 1. Vaccination Variables (I) ---
    for (int i = 0; i < N; i++) {
        double lb = 0.0, ub = 1.0;
        
        if (pre_vaccinated_mask[i]) {
            // Already vaccinated: Force 1
            lb = 1.0; ub = 1.0; 
        } else if (already_infected_mask[i]) {
            // Already infected: Force 0 (Cannot waste vaccine)
            lb = 0.0; ub = 0.0;
        }
        
        lp.I[i] = lp.model.addVar(lb, ub, 0.0, GRB_CONTINUOUS);
    }

    // --- 2. State Variables (x) ---
    for (int i = 0; i < N; i++) {
        for (int s = 0; s < S; s++) {
            // Obj coeff: 1.0/S (Minimize expected infection)
            if(already_infected_mask[i]) {
                // Already infected: Force 1
                lp.x[i][s] = lp.model.addVar(1.0, 1.0, 1.0 / S, GRB_CONTINUOUS);
            } else{
                lp.x[i][s] = lp.model.addVar(0.0, 1.0, 1.0 / S, GRB_CONTINUOUS);
            }
        }
    }
    lp.model.update();

    // --- 3. Initial Infection Constraints ---
    for (int s = 0; s < S; s++) {
        for (int i : current_infected) {
            lp.model.addConstr(lp.x[i][s] == 1.0);
        }
    }

    // --- 4. Spread Constraints (Cut-based) ---
    for (int s = 0; s < S; s++) {
        const AdjList &adj = samples[s];
        for (int j = 0; j < N; j++) {
            for (int i : adj[j]) {
                // If neighbor j is infected, i gets infected unless vaccinated
                // x[i] >= x[j] - I[i]  =>  x[i] - x[j] + I[i] >= 0
                lp.model.addConstr(lp.x[i][s] - lp.x[j][s] + lp.I[i] >= 0.0);
            }
        }
    }

    // --- 5. Budget Constraint ---
    // Sum of ALL vaccinations (past + present) <= Cumulative Budget
    GRBLinExpr total_vacc = 0;
    for (int i = 0; i < N; i++) total_vacc += lp.I[i];
    lp.model.addConstr(total_vacc <= total_cumulative_budget);

    lp.model.update();
    return lp;
}

// Extracts best candidates using Iterative Rounding (IRP) Logic adapted for Rolling Horizon
static std::vector<int> solve_lp_and_pick_new(
    LPModel &lp, 
    int marginal_budget,
    const std::vector<bool>& pre_vaccinated_mask,
    const std::vector<bool>& already_infected_mask
) {
    lp.model.optimize();
    
    // Get scores for all nodes
    std::vector<double> scores(lp.N);
    for (int i = 0; i < lp.N; i++) {
        scores[i] = lp.I[i].get(GRB_DoubleAttr_X);
    }

    // Sort available nodes by score
    std::vector<int> candidates;
    candidates.reserve(lp.N);
    for(int i=0; i<lp.N; ++i) {
        // Only consider nodes that are VALID (not vaccinated, not infected)
        if(!pre_vaccinated_mask[i] && !already_infected_mask[i]) {
            candidates.push_back(i);
        }
    }

    std::sort(candidates.begin(), candidates.end(), [&](int a, int b) {
        return scores[a] > scores[b];
    });

    // Pick top 'marginal_budget' nodes
    std::vector<int> result;
    for(int i=0; i < std::min((int)candidates.size(), marginal_budget); ++i) {
        result.push_back(candidates[i]);
    }
    return result;
}



std::vector<std::pair<int,int>> run_rolling_horizon_strategy(
    Graph &base_graph,                  
    std::vector<int> initial_infected,
    const std::vector<int> &budget_schedule, 
    int time_step_gap,
    int num_samples_per_step,
    double prob_infect
) { 
    // Copy the graph before mutating it 
    Graph::Params params_copy = base_graph.params;
    Graph Gc(params_copy, 42); // Seed doesn't matter here

    Gc.centers = base_graph.centers;
    Gc.nodes = base_graph.nodes;
    Gc.adj_list = base_graph.adj_list;
    Gc.nodesThreshold = base_graph.nodesThreshold;
    Gc.build_spatial_index();

    int N = Gc.nodes.size();
    
    // Global State
    std::vector<bool> vaccinated_mask(N, false);
    // This tracks the "Real World" infection state
    std::vector<int> current_infected_list = initial_infected; 

    std::vector<std::pair<int,int>> all_vaccinated_nodes;
    int current_cumulative_budget = 0;
    int time_id = 0 ; 

    for (size_t t = 0; t < budget_schedule.size(); ++t) {
        int marginal_budget = budget_schedule[t];
        current_cumulative_budget += marginal_budget;

        // Sync masks for the LP
        std::vector<bool> already_infected_mask(N, false);
        for(int u : current_infected_list) already_infected_mask[u] = true;

        std::cout << ">>> Time Step " << t 
                  << " | Current Infected (Real): " << current_infected_list.size() 
                  << " | Budget: " << marginal_budget << std::endl;

        // 1. SAMPLE: Generate topologies (Prediction of connectivity)
        auto samples = sample_ic_live_edge_topologies(Gc, num_samples_per_step, 42 + t, prob_infect);

        // 2. OPTIMIZE: Run LP to find best NEW vaccines
        LPModel lp = build_lp_model(
            samples, 
            current_infected_list, 
            vaccinated_mask, 
            already_infected_mask, 
            current_cumulative_budget
        );

        // 3. DECIDE: Extract top candidates
        // Note: This is "Simple Rounding". For better quality, use Iterative Rounding here.
        std::vector<int> new_vaccines = solve_lp_and_pick_new(
            lp, 
            marginal_budget, 
            vaccinated_mask, 
            already_infected_mask
        );

        // 4. ACT: Apply vaccines
        for (int v : new_vaccines) {
            vaccinated_mask[v] = true;
            all_vaccinated_nodes.push_back({v,time_id});
        }
        std::cout << "    Vaccinated " << new_vaccines.size() << " nodes." << std::endl;

        // If no new vaccinations, returnearly
        if(new_vaccines.empty() && marginal_budget > 0) {
            std::cout << "No new vaccinations possible. Ending early.\n";
            break;
        }

        // 5. OBSERVE: Simulate "Reality" 
        // We use the base_graph (or a specific realization) to roll the dice 
        // and see who actually gets sick before the next decision.
        simulate_infection_spread(
            Gc,             // The "Environment"
            current_infected_list,  // Input/Output: Will expand based on IC model
            vaccinated_mask,        // Vaccines protect nodes
            time_step_gap,          // How long we wait
            100 + t ,                // Random seed
            prob_infect
        );

        // 6. EVOLVE: Move Nodes
        Gc.simulate_movement(time_step_gap);
        time_id  += time_step_gap ; // Increaseing the time for next vaccination record 
    }

    return all_vaccinated_nodes;
}