#include "waxman-graph.h"
#include "dynamic-strategy.h"
#include "dynamic-simulation.h"
#include "config.h"
#include <set>
#include "simulation.h"
#include <chrono>
#include <iomanip>
#include <sstream> 
#include "main-helper.h"

double Prob_infect = 0.5 ; 

int main() {
    std::cout << "Program started: Dynamic LP (IRP) Vaccination (Default Config)\n";
    
    Config cfg;
    if (!load_config("config/config-lpirp.txt", cfg)) {
        std::cerr << "Failed to load config/config-lpirp.txt\n";
        return 1;
    }

    Prob_infect = cfg.prob_infect ;

    // Open the CSV file for writing resultsstd::ostringstream pct;
    std::ostringstream pct;
    pct << std::fixed << std::setprecision(2) << cfg.vaccination_budget_percent;

    std::filesystem::create_directories("results");
    std::filesystem::create_directories("results/dynamic_lpirp");

    std::ofstream csv(
        "results/dynamic_lpirp/nodes_saved_" + pct.str() + ".csv"
    );
    std::ofstream csv_time(
        "results/dynamic_lpirp/time_taken_" + pct.str() + ".csv"
    );
    csv << "NodeSize,Uniform,FrontLoaded,BackLoaded,StaticOneShot,WithoutVaccine\n";
    csv << std::fixed << std::setprecision(2);
    csv_time << "NodeSize,Uniform,FrontLoaded,BackLoaded,StaticOneShot,WithoutVaccine\n";
    csv_time << std::fixed << std::setprecision(2);

    for (int N : cfg.node_sizes){
                
        int total_budget = static_cast<int>(cfg.vaccination_budget_percent * N);
        int time_interval = cfg.timegap;
        int batches = static_cast<int>(0.5 * std::log2(static_cast<double>(N)));
        cfg.batches = std::clamp(batches, 3, 20);
        std::cout << "Batches set to: " << cfg.batches << "\n";

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
            std::vector<std::pair<int,int>> vaccines = run_rolling_horizon_strategy_irp(
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

            // Extract only the vaccines 
            std::vector<int> only_vaccine = extractVaccineNode(vaccines) ; 

            int saved_count = run_baseline_with_seeds(
                G,
                initial_infected,
                //only_vaccine, 
                vaccines,
                IC_Simulation_test,
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
            IC_Simulation_test,
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