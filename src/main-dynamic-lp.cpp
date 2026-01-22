#include "waxman-graph.h"
#include "dynamic-strategy.h"
#include "config.h"
#include <set>
#include "simulation.h"
#include <chrono>
#include <iomanip>
#include <sstream> 

double Prob_infect = 0.5 ; 

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

enum class ScheduleType {
    UNIFORM,
    FRONT_LOADED, // Heavy initial vaccination
    BACK_LOADED,  // Save resources for later
    STATIC_ONE_SHOT // Apply all at t=0
};

std::vector<int> generate_budget_schedule(int total_k, int steps, ScheduleType type) {
    std::vector<int> schedule(steps, 0);
    if (type == ScheduleType::STATIC_ONE_SHOT) {
        schedule.resize(1) ; 
        schedule[0] = total_k; // All at once
        return schedule;
    }

    if (type == ScheduleType::UNIFORM) {
        int base = total_k / steps;
        int remainder = total_k % steps;
        for (int i = 0; i < steps; i++) {
            schedule[i] = base + (i < remainder ? 1 : 0);
        }
    } 
    else if (type == ScheduleType::FRONT_LOADED) {
        int current_k = total_k;
        for(int i=0; i<steps-1; ++i) {
            int alloc = std::max(1, (int)(current_k * 0.5)); // 50% of remaining
            schedule[i] = alloc;
            current_k -= alloc;
        }
        schedule[steps-1] = current_k; // Dump remainder
    }
    else if (type == ScheduleType::BACK_LOADED) {
        std::vector<int> temp = generate_budget_schedule(total_k, steps, ScheduleType::FRONT_LOADED);
        std::reverse(temp.begin(), temp.end());
        schedule = temp;
    }
    return schedule;
}

struct StrategyResult {
    int saved_count;
    double time_ms;
    std::string strategy_name;
};

using SimulatorFn = std::function<int(
    Graph &,
    const std::vector<bool> &,
    const int &,
    const std::vector<bool> &,
    const uint64_t &,
    const int &,
    const double
)>;

static int run_baseline(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<int> &initial_vaccinated,
    SimulatorFn Simulator,
    uint64_t seed,
    int stepSize,
    double prob_infect) {
    int N = G.nodes.size();
    std::vector<bool> vaccinated(N, false);
    std::vector<bool> infected(N, false);

    for (int u : initial_infected)
        infected[u] = true;

    for (int u : initial_vaccinated)
        vaccinated[u] = true;

    int saved_cnt = Simulator(G, vaccinated, -1, infected, seed, stepSize, prob_infect); // saved nodes 
    return saved_cnt; // return number of saved nodes 
}

static int run_baseline_with_seeds(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<int> &initial_vaccinated,
    SimulatorFn Simulator,
    const std::vector<uint64_t> &seeds,
    int stepSize,
    double prob_infect) {

    int total = 0;
    for (uint64_t s : seeds) {
        total += run_baseline(
            G,
            initial_infected,
            initial_vaccinated,
            Simulator,
            s,
            stepSize,
            prob_infect
        );
    }

    int avg = total / seeds.size();
    return avg; // return average infected nodes
}

void scale_waxman_params(int N, double base_alpha, double base_beta, double &out_alpha, double &out_beta) {
    const double N_REF = 256.0; 
    out_alpha = base_alpha;
    double scale_factor = std::sqrt(N_REF) / std::sqrt((double)N);
    out_beta = base_beta * scale_factor;
}

int main() {
    std::cout << "Program started: Dynamic Greedy Vaccination (Default Config)\n";
    
    Config cfg;
    if (!load_config("config.txt", cfg)) {
        std::cerr << "Failed to load config.txt\n";
        return 1;
    }

    Prob_infect = cfg.prob_infect ;

    // Open the CSV file for writing resultsstd::ostringstream pct;
    std::ostringstream pct;
    pct << std::fixed << std::setprecision(2) << cfg.vaccination_budget_percent;

    std::ofstream csv(
        "results/dynamic_lp_nodes_saved_" + pct.str() + ".csv"
    );
    std::ofstream csv_time(
        "results/dynamic_lp_time_taken_" + pct.str() + ".csv"
    );
    csv << "NodeSize,Uniform,FrontLoaded,BackLoaded,StaticOneShot,WithoutVaccine\n";
    csv << std::fixed << std::setprecision(2);
    csv_time << "NodeSize,Uniform,FrontLoaded,BackLoaded,StaticOneShot,WithoutVaccine\n";
    csv_time << std::fixed << std::setprecision(2);

    for (int N : cfg.node_sizes){
                
        int total_budget = static_cast<int>(cfg.vaccination_budget_percent * N);
        int time_interval = cfg.timegap;

        std::cout << "\n========================================\n";
        std::cout << "Experiment start\n";
        std::cout << "N = " << N
                << " | alpha = " << cfg.alpha
                << " | beta = " << cfg.beta
                << " | cutoff = " << cfg.cutoff_prob << "\n";
        std::cout << "Initial infected fraction = "
                << cfg.initial_infected_percent << "\n";
        std::cout << "Vaccination budget fraction = "
                << cfg.vaccination_budget_percent
                << " (total k = " << total_budget << ")\n";
        std::cout << "Batches = " << cfg.batches
                << " | Time gap = " << time_interval
                << " | IC stepSize = " << cfg.stepSize << "\n";
        std::cout << "========================================\n";
            
        Graph::Params p;
        p.num_cities = std::max(2, (int)std::sqrt(N) / 2);
        p.num_villages = 3*p.num_cities;
        double world_dim = std::sqrt(N)*2.0 ;
        p.world_min_x = 0; p.world_max_x = world_dim;
        p.world_min_y = 0; p.world_max_y = world_dim;
        p.nodes_per_city = static_cast<int>(0.7 * N / p.num_cities);
        p.nodes_per_village = (N - p.nodes_per_city * p.num_cities) / p.num_villages;
        scale_waxman_params(N, cfg.alpha, cfg.beta, p.alpha, p.beta);
        p.cutoff_prob = cfg.cutoff_prob;
        if(p.nodes_per_village <= 0) p.nodes_per_village = 0;
    
        std::cout << "Generating Graph...\n";
        Graph G(p, cfg.seed);
        G.generate_centers();
        G.generate_nodes();
        G.build_spatial_index();
        G.generate_edges();
        std::cout << "Graph generated.\n";
        std::cout << "  Nodes: " << G.nodes.size() << "\n";
        std::cout << "  Avg degree: ";

        long long edge_sum = 0;
        for (const auto &nbrs : G.adj_list) edge_sum += nbrs.size();
        std::cout << (double)edge_sum / G.nodes.size() << "\n";
        std::cout << "Edges count : " << edge_sum / 2 << "\n";

        std::vector<int> initial_infected = sample_initial_infected(G, cfg.initial_infected_percent, cfg.seed);
        std::cout << "Initial infected nodes sampled: "
          << initial_infected.size() << "\n";

        std::vector<StrategyResult> results;
        std::vector<uint64_t> seeds ;
        for(int i=0; i<cfg.T; ++i){
            seeds.push_back(cfg.seed + i);
        }

        for(auto batchType : {ScheduleType::UNIFORM, ScheduleType::FRONT_LOADED, ScheduleType::BACK_LOADED, ScheduleType::STATIC_ONE_SHOT}){
            std::vector<int> schedule = generate_budget_schedule(total_budget, cfg.batches, batchType);
            auto t1 = std::chrono::high_resolution_clock::now();
            std::vector<int> vaccines = run_rolling_horizon_strategy(
                G,
                initial_infected,
                schedule,
                time_interval,
                cfg.T, // samples per step
                Prob_infect
            );
            auto t2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = t2 - t1;
            std::cout << "Total vaccines selected: "
            << vaccines.size() << "\n";

            int saved_count = run_baseline_with_seeds(
                G,
                initial_infected,
                vaccines,
                IC_Simulation,
                seeds,
                cfg.stepSize,
                Prob_infect
            );

            results.push_back({saved_count, elapsed.count(),
                (batchType == ScheduleType::UNIFORM) ? "UNIFORM" :
                (batchType == ScheduleType::FRONT_LOADED) ? "FRONT_LOADED" :
                (batchType == ScheduleType::BACK_LOADED) ? "BACK_LOADED" :
                "STATIC_ONE_SHOT"
            });

            std::cout << "N = " << N << " | Strategy " << results.back().strategy_name
                      << " | Saved: " << saved_count
                      << " | Time: " << elapsed.count() << " s\n";
        }

        // Result without vaccination
        auto t3 = std::chrono::high_resolution_clock::now();
        int saved_no_vaccine = run_baseline_with_seeds(
            G,
            initial_infected,
            {},
            IC_Simulation,
            seeds,
            cfg.stepSize,
            Prob_infect
        );
        auto t4 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_no_vaccine = t4 - t3;

        // Write the results to csv 
        csv << p.nodes_per_city * p.num_cities + p.nodes_per_village * p.num_villages << ",";
        csv_time << p.nodes_per_city * p.num_cities + p.nodes_per_village * p.num_villages << ",";
        for(auto result : results){
            csv << result.saved_count << ",";
            csv_time << result.time_ms << ",";
        }
        csv << saved_no_vaccine << "\n";
        csv_time << elapsed_no_vaccine.count() << "\n";
    }

    return 0;
}