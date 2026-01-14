// ------------------------------------------------------------
// src/benchmark-stepsize.cpp
//
// Percentage of nodes saved vs stepSize
// for Gaussian Waxman graphs
// ------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <functional>
#include <iomanip>
#include <cstdint>
#include <cassert>
#include <chrono>
#include <unordered_set>

#include "waxman-graph.h"
#include "strategy.h"
#include "simulation.h"
#include "sampler.h"
#include "config.h"

// ------------------------------------------------------------
// Debug macro
// ------------------------------------------------------------
// #define DEBUG  

#ifdef DEBUG
#define DBG(msg) \
    do { std::cerr << "[DBG] " << msg << std::endl; } while (0)
#else
#define DBG(msg) do {} while (0)
#endif

using SimulatorFn = std::function<int(
    Graph &,
    const std::vector<bool> &,
    const int &,
    const std::vector<bool> &,
    const uint64_t &,
    const int &
)>;

using Clock = std::chrono::high_resolution_clock;
using ms = std::chrono::duration<double, std::milli>;

// ------------------------------------------------------------
// Seed generation
// ------------------------------------------------------------
std::vector<uint64_t> generate_seeds(uint64_t seed, int T) {
    DBG("Generating " << T << " seeds");
    std::vector<uint64_t> seeds;
    for (int i = 0; i < T; ++i) {
        uint64_t s = seed + 1337 + i * 9973;
        seeds.push_back(s);
        DBG("  seed[" << i << "] = " << s);
    }
    return seeds;
}

// ------------------------------------------------------------
// Baseline runner (returns infected count)
// ------------------------------------------------------------
static int run_baseline(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<int> &initial_vaccinated,
    SimulatorFn Simulator,
    uint64_t seed,
    int stepSize) {

    DBG("run_baseline | seed=" << seed << " stepSize=" << stepSize);

    int N = G.nodes.size();
    std::vector<bool> vaccinated(N, false);
    std::vector<bool> infected(N, false);

    for (int u : initial_infected)
        infected[u] = true;

    for (int u : initial_vaccinated)
        vaccinated[u] = true;

    int infected_cnt = Simulator(G, vaccinated, -1, infected, seed, stepSize);
    DBG("run_baseline | infected=" << infected_cnt
        << " saved=" << (N - infected_cnt));

    return N - infected_cnt;
}

static int run_baseline_with_seeds(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<int> &initial_vaccinated,
    SimulatorFn Simulator,
    const std::vector<uint64_t> &seeds,
    int stepSize) {

    DBG("run_baseline_with_seeds | stepSize=" << stepSize
        << " seeds=" << seeds.size());

    int total = 0;
    for (uint64_t s : seeds) {
        total += run_baseline(
            G,
            initial_infected,
            initial_vaccinated,
            Simulator,
            s,
            stepSize
        );
    }

    int avg = total / seeds.size();
    DBG("run_baseline_with_seeds | avg saved=" << avg);
    return avg;
}

// ------------------------------------------------------------
// Initial infected sampling
// ------------------------------------------------------------
std::vector<int> sample_initial_infected(
    const Graph &G,
    double frac,
    uint64_t seed) {

    int N = G.nodes.size();
    int k = static_cast<int>(frac * N);

    DBG("Sampling initial infected | N=" << N
        << " frac=" << frac
        << " k=" << k
        << " seed=" << seed);

    std::set<int> s;
    Random_number_generator rng(seed);

    while ((int)s.size() < k) {
        s.insert(rng.get_int(0, N - 1));
    }

    DBG("Initial infected sampled: " << s.size());
    return std::vector<int>(s.begin(), s.end());
}

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
int main() {

    DBG("Program started: benchmark-stepsize");

    Config cfg;
    if (!load_config("config.txt", cfg)) {
        std::cerr << "Failed to load config.txt\n";
        return 1;
    }

    DBG("Config loaded");
    DBG("Diffusion model = " << cfg.diffusion_model);
    DBG("T = " << cfg.T
        << ", budget = " << cfg.vaccination_budget_percent
        << ", init infected = " << cfg.initial_infected_percent);

    SimulatorFn Simulator =
        (cfg.diffusion_model == "LT")
            ? LinearThreshold_Simulator
            : IC_Simulation;

    std::vector<int> step_sizes = {3};

    std::ofstream csv("results/benchmark_stepsize_" + std::to_string(cfg.node_sizes.front()) + ".csv");
    csv << "stepsize,baseline,lp_tkr,lp_irp,pg_gaurd_neigh,pg_sort_topk\n";
    csv << std::fixed << std::setprecision(2);
    // std::ofstream csv("results/benchmark_stepsize_" + std::to_string(cfg.node_sizes.front()) + ".csv");
    // csv << "stepsize,baseline,greedy,local_search,hill_climbing,lp_tkr,lp_irp,pg_gaurd_neigh,pg_sort_topk\n";
    // csv << std::fixed << std::setprecision(2);

    std::ofstream csv_time(
        "results/benchmark_stepsize_time_" +
        std::to_string(cfg.node_sizes.front()) + ".csv"
    );
    csv_time << "stepsize,baseline,lp_tkr,lp_irp,pg_gaurd_neigh,pg_sort_topk\n";
    csv_time << std::fixed << std::setprecision(3);
    // std::ofstream csv_time(
    //     "results/benchmark_stepsize_time_" +
    //     std::to_string(cfg.node_sizes.front()) + ".csv"
    // );
    // csv_time << "stepsize,baseline,greedy,local_search,hill_climbing,lp_tkr,lp_irp,pg_gaurd_neigh,pg_sort_topk\n";
    // csv_time << std::fixed << std::setprecision(3);

    int N = cfg.node_sizes.front();
    DBG("Fixed N = " << N);

    // ---------------- Graph params ----------------
    Graph::Params p;
    p.num_cities = cfg.num_centers;
    p.num_villages = cfg.num_centers * 2;
    p.nodes_per_city = static_cast<int>(0.7 * N / cfg.num_centers);
    p.nodes_per_village =
        (N - p.nodes_per_city * cfg.num_centers) / p.num_villages;
    p.alpha = cfg.alpha;
    p.beta = cfg.beta;
    p.cutoff_prob = cfg.cutoff_prob;

    DBG("Graph parameters set");

    Graph G(p, cfg.seed);
    G.generate_centers();
    G.generate_nodes();
    G.build_spatial_index();
    G.generate_edges();

    DBG("Graph constructed | nodes=" << G.nodes.size()
        << " edgesets=" << G.adj_list.size());

    for (int stepSize : step_sizes) {

        DBG("======================================");
        DBG("BEGIN stepSize = " << stepSize);

        int k = static_cast<int>(cfg.vaccination_budget_percent * N);
        DBG("Vaccination budget k = " << k);

        auto seeds = generate_seeds(cfg.seed, cfg.T);
        auto initial_infected =
            sample_initial_infected(G, cfg.initial_infected_percent, cfg.seed);
        
        // Find the # of unnique neighbor set for the infected nodes 
        std::unordered_set<int> unique_neighbors;
        for(int i : initial_infected){
            for(int v : G.adj_list[i]){
                unique_neighbors.insert(v);
            }
        }
        std::cout << "StepSize=" << stepSize 
                 << " Nodes generated=" << N
                  << " Initial Infected Nodes=" << initial_infected.size() 
                  << " Unique Neighbor Nodes=" << unique_neighbors.size() 
                  << "Vaccination Budget k=" << k << std::endl;
        DBG("Initial infected nodes: " << initial_infected.size());
        DBG("Unique neighbors of initial infected nodes: " << unique_neighbors.size());

        DBG("---- Baseline ----");
        auto t0 = Clock::now();
        int base_saved =
            run_baseline_with_seeds(
                G, initial_infected, {},
                Simulator, seeds, stepSize
            );
        auto t1 = Clock::now();
        double base_time = ms(t1 - t0).count();
        double base_pct = (N - (double)base_saved) / N * 100.0;
        DBG("Baseline time : " << base_time << " ms");
        // std::cout << "Baseline time : " << base_time << " ms" << std::endl;
        // std::cout << "Baseline saved count: " << (N - base_saved) << std::endl;

        // DBG("---- Greedy ----");
        // t0 = Clock::now();
        // std::vector<int> greedy;
        // Greedy_Vaccination(
        //     G, k, initial_infected,
        //     greedy, Simulator,
        //     stepSize, cfg.T
        // );
        // t1 = Clock::now();
        // double greedy_time = ms(t1 - t0).count();
        // int greedy_saved =
        //     run_baseline_with_seeds(
        //         G, initial_infected, greedy,
        //         Simulator, seeds, stepSize
        //     );
        // double greedy_pct = (N - (double)greedy_saved) / N * 100.0;
        // DBG("Greedy time : " << greedy_time << " ms");
        // std::cout << "Greedy time : " << greedy_time << " ms" << std::endl;

        // DBG("---- Local Search ----");
        // t0 = Clock::now();
        // std::vector<int> ls;
        // Local_Search(
        //     G, k, initial_infected,
        //     ls, Simulator,
        //     stepSize, cfg.T, 50, greedy
        // );
        // t1 = Clock::now();
        // double ls_time = ms(t1 - t0).count();
        // int ls_saved =
        //     run_baseline_with_seeds(
        //         G, initial_infected, ls,
        //         Simulator, seeds, stepSize
        //     );
        // double ls_pct = ( N - (double)ls_saved) / N * 100.0;
        // DBG("Local Search time : " << ls_time << " ms");
        // std::cout << "Local Search time : " << ls_time << " ms" << std::endl;

        // DBG("---- Hill Climbing ----");
        // std::vector<int> hc;
        // t0 = Clock::now();
        // HIll_Climbing(
        //     G, k, initial_infected,
        //     hc, Simulator,
        //     stepSize, cfg.T, 10, greedy
        // );
        // t1 = Clock::now();
        // double hc_time = ms(t1 - t0).count();
        // int hc_saved =
        //     run_baseline_with_seeds(
        //         G, initial_infected, hc,
        //         Simulator, seeds, stepSize
        //     );
        // double hc_pct = ( N - (double)hc_saved) / N * 100.0;
        // DBG("Hill Climbing time : " << hc_time << " ms");
        // std::cout << "Hill Climbing time : " << hc_time << " ms" << std::endl;

        DBG("---- LP Sampling ----");
        t0 = Clock::now();
        std::vector<Graph> samples =
            (cfg.diffusion_model == "LT")
                ? sample_lt_live_edge_topologies(G, cfg.T, cfg.seed)
                : sample_ic_live_edge_topologies(G, cfg.T, cfg.seed);
        DBG("LP samples = " << samples.size());
        t1 = Clock::now();
        double lp_sample_time = ms(t1 - t0).count();
        DBG("Time for LP sampling : " << lp_sample_time << " ms");
        std::cout << "Time for LP sampling : " << lp_sample_time << " ms" << std::endl;

        DBG("---- LP-TKR ----");
        t0 = Clock::now();
        auto lp_tkr =
            solve_lp_vaccination(samples, initial_infected, k, RoundingMethod::TKR);
        t1 = Clock::now();
        double lp_tkr_time = ms(t1 - t0).count();
        int lp_tkr_saved =
            run_baseline(
                G, initial_infected, lp_tkr,
                Simulator, cfg.seed, stepSize
            );
        double lp_tkr_pct = (N - (double)lp_tkr_saved) / N * 100.0;
        DBG("Time for LP-TKR : " << lp_tkr_time << " ms");
        std::cout << "Time for LP-TKR : " << lp_tkr_time << " ms" << std::endl;
        std::cout << "LP-TKR saved saved: " << (N - lp_tkr_saved) << std::endl;

        DBG("---- LP-IRP ----");
        t0 = Clock::now();
        auto lp_irp =
            solve_lp_vaccination(samples, initial_infected, k, RoundingMethod::IRP);
        t1 = Clock::now();
        double lp_irp_time = ms(t1 - t0).count();
        int lp_irp_saved =
            run_baseline_with_seeds(
                G, initial_infected, lp_irp,
                Simulator, seeds, stepSize
            );
        double lp_irp_pct = (N - (double)lp_irp_saved) / N * 100.0;
        DBG("Time for LP-IRP : " << lp_irp_time << " ms");
        std::cout << "Time for LP-IRP : " << lp_irp_time << " ms" << std::endl;

        DBG("---PageRank Results---");
        t0 = Clock::now();
        auto pg_m1 = 
            PageRank(G, k, initial_infected, 0.85, 1e-6, 100, true);
        t1 = Clock::now();
        double pg_m1_time = ms(t1 - t0).count();
        int pg_m1_saved =
            run_baseline_with_seeds(
                G, initial_infected, pg_m1,
                Simulator, seeds, stepSize
            );
        double pg_m1_pct = (N - (double)pg_m1_saved) / N * 100.0;
        DBG("PageRank (gaurd the top infected node) saved pct: " << pg_m1_pct);
        DBG("Time for PageRank (gaurd the top infected node) : " << pg_m1_time << " ms");
        std::cout << "Time for PageRank (gaurd the top infected node) : " << pg_m1_time << " ms" << std::endl;

        DBG("---PageRank Results---");
        t0 = Clock::now();
        auto pg_m2 = 
            PageRank(G, k, initial_infected, 0.85, 1e-6, 100, false);
        t1 = Clock::now();
        double pg_m2_time = ms(t1 - t0).count();
        int pg_m2_saved =
            run_baseline_with_seeds(
                G, initial_infected, pg_m2,
                Simulator, seeds, stepSize
            );
        double pg_m2_pct = (N - (double)pg_m2_saved) / N * 100.0;
        DBG("PageRank (sort topk) saved pct: " << pg_m2_pct);    
        DBG("Time for PageRank (sort topk) : " << pg_m2_time << " ms");
        std::cout << "Time for PageRank (sort topk) : " << pg_m2_time << " ms" << std::endl;
        
        csv << stepSize << ","
            << base_pct << ","
            << lp_tkr_pct << ","
            << lp_irp_pct << "," 
            << pg_m1_pct << ","
            << pg_m2_pct << "\n";
        
        csv_time << stepSize << ","
         << base_time << ","
         << (lp_sample_time + lp_tkr_time) << ","
         << (lp_sample_time + lp_irp_time) << ","
         << pg_m1_time << ","
         << pg_m2_time << "\n";

        std::cout << "Nodes More saved than baseline (apart from vaccinated) : " << 
            ( (N - lp_tkr_saved) - (N - base_saved) ) << std::endl;

        // csv << stepSize << ","
        //     << base_pct << ","
        //     << greedy_pct << ","
        //     << ls_pct << ","
        //     << hc_pct << ","
        //     << lp_tkr_pct << ","
        //     << lp_irp_pct << "," 
        //     << pg_m1_pct << ","
        //     << pg_m2_pct << "\n";
        
        // csv_time << stepSize << ","
        //  << base_time << ","
        //  << greedy_time << ","
        //  << ls_time << ","
        //  << hc_time << ","
        //  << (lp_sample_time + lp_tkr_time) << ","
        //  << (lp_sample_time + lp_irp_time) << ","
        //  << pg_m1_time << ","
        //  << pg_m2_time << "\n";

        DBG("END stepSize = " << stepSize);
    }

    csv.close();
    DBG("benchmark-stepsize finished successfully");
    return 0;
}


// Total Count - 2048 , Baseline Infected - 739 , Vaccinated Infected - 457 , Vaccninated Nodes - 204 , Extra saved by vaccination - 282 , More : 78 
