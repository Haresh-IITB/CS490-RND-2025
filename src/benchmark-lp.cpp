// ------------------------------------------------------------
// src/benchmark-vaccination-lp.cpp  (DEBUG BUILD)
//
// Average number of nodes saved by LP-based vaccination
// algorithms (TKR + IRP) for Gaussian Waxman graphs
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

#include "waxman-graph.h"
#include "strategy.h"
#include "simulation.h"
#include "sampler.h"
#include "config.h"

// ------------------------------------------------------------
// Debug macro (flushes immediately)
// ------------------------------------------------------------
#define DEBUG 

#ifdef DEBUG
#define DBG(msg) \
    do { std::cerr << "[DBG] " << msg << std::endl; } while (0)
#else
#define DBG(msg) do {} while (0)
#endif

using Clock = std::chrono::steady_clock;

static double elapsed_seconds(
    const Clock::time_point &start,
    const Clock::time_point &end
) {
    return std::chrono::duration<double>(end - start).count();
}

using SimulatorFn = std::function<int(
    Graph &,
    const std::vector<bool> &,
    const int &,
    const std::vector<bool> &,
    const uint64_t &,
    const int &
)>;

// ------------------------------------------------------------
// Seed generation
// ------------------------------------------------------------
std::vector<uint64_t> generate_seeds(uint64_t seed, int T) {
    DBG("Generating " << T << " seeds");
    std::vector<uint64_t> seeds;
    for (int i = 0; i < T; i++) {
        seeds.push_back(seed + 1337 + i * 9973);
    }
    return seeds;
}

// ------------------------------------------------------------
// Simulator wrapper
// ------------------------------------------------------------
static int run_baseline(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<int> &initial_vaccinated,
    SimulatorFn Simulator,
    uint64_t seed,
    int stepSize) {

    int N = G.nodes.size();
    std::vector<bool> vaccinated(N, false);
    std::vector<bool> infected(N, false);

    for (int u : initial_infected) infected[u] = true;
    for (int u : initial_vaccinated) vaccinated[u] = true;

    int newVaccinatedNode = -1;
    return N - Simulator(G, vaccinated, newVaccinatedNode, infected, seed, stepSize); // return number of infected nodes 
}

// ------------------------------------------------------------
// Averaged baseline
// ------------------------------------------------------------
static int run_baseline_with_seeds(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<int> &initial_vaccinated,
    SimulatorFn Simulator,
    const std::vector<uint64_t> &seeds,
    int stepSize) {

    int total = 0;
    for (uint64_t s : seeds) {
        total += run_baseline(
            G, initial_infected, initial_vaccinated,
            Simulator, s, stepSize
        );
    }
    return total / seeds.size(); // returns average number of infected nodes
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

    std::set<int> s;
    Random_number_generator rng(seed);

    while ((int)s.size() < k)
        s.insert(rng.get_int(0, N - 1));

    return std::vector<int>(s.begin(), s.end());
}

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
int main() {

    DBG("Program started (LP-only)");

    Config cfg;
    if (!load_config("config.txt", cfg)) {
        std::cerr << "Failed to load config.txt\n";
        return 1;
    }

    SimulatorFn Simulator =
        (cfg.diffusion_model == "LT")
            ? LinearThreshold_Simulator
            : IC_Simulation;

    std::ofstream csv("results/benchmark_lp_results.csv");
    csv << "nodes,lp_tkr,lp_irp\n";
    csv << std::fixed << std::setprecision(2);

    std::ofstream csv_time("results/benchmark_lp_time.csv");
    csv_time << "nodes,lp_tkr,lp_irp\n";
    csv_time << std::fixed << std::setprecision(6);

    // --------------------------------------------------------
    for (int N : cfg.node_sizes) {

        double t_lp_tkr = 0.0;
        double t_lp_irp = 0.0;

        DBG("======================================");
        DBG("Starting LP-only experiment for N = " << N);

        // ---------------- Graph params ----------------
        Graph::Params p;
        p.num_cities   = cfg.num_centers;
        p.num_villages = cfg.num_centers * 2;

        p.nodes_per_city =
            static_cast<int>(0.7 * N / cfg.num_centers);

        p.nodes_per_village =
            (N - p.nodes_per_city * cfg.num_centers) / p.num_villages;

        p.alpha        = cfg.alpha;
        p.beta         = cfg.beta;
        p.cutoff_prob  = cfg.cutoff_prob;

        Graph G(p, cfg.seed);
        G.generate_centers();
        G.generate_nodes();
        G.build_spatial_index();
        G.generate_edges();

        int k = static_cast<int>(cfg.vaccination_budget_percent * N);

        auto seeds = generate_seeds(cfg.seed, cfg.T);
        auto initial_infected =
            sample_initial_infected(G, cfg.initial_infected_percent, cfg.seed);

        // ---------------- LP Sampling ----------------
        DBG("Sampling LP live-edge graphs");
        std::vector<Graph> samples =
            (cfg.diffusion_model == "LT")
                ? sample_lt_live_edge_topologies(G, cfg.T, cfg.seed)
                : sample_ic_live_edge_topologies(G, cfg.T, cfg.seed);

        // ---------------- LP-TKR ----------------
        DBG("Running LP-TKR");
        auto t0 = Clock::now();
        auto lp_tkr =
            solve_lp_vaccination(samples, initial_infected, k, RoundingMethod::TKR);
        auto t1 = Clock::now();
        t_lp_tkr = elapsed_seconds(t0, t1);

        int lp_tkr_inf =
            run_baseline_with_seeds(
                G, initial_infected, lp_tkr,
                Simulator, seeds, cfg.stepSize
            );

        // ---------------- LP-IRP ----------------
        DBG("Running LP-IRP");
        t0 = Clock::now();
        auto lp_irp =
            solve_lp_vaccination(samples, initial_infected, k, RoundingMethod::IRP);
        t1 = Clock::now();
        t_lp_irp = elapsed_seconds(t0, t1);

        int lp_irp_inf =
            run_baseline_with_seeds(
                G, initial_infected, lp_irp,
                Simulator, seeds, cfg.stepSize
            );
        
        // Baseline saved 
        int baseline_saved = N - run_baseline_with_seeds(
            G, initial_infected, {},
            Simulator, seeds, cfg.stepSize
        );

        std::cout << "N=" << N
                  << " lp_tkr_saved=" << N - lp_tkr_inf
                  << " lp_irp_saved=" << N - lp_irp_inf
                  << " baseline_saved=" << baseline_saved
                  << std::endl;

        csv << N << ","
            << N - lp_tkr_inf << ","
            << N - lp_irp_inf << ","
            << baseline_saved << "\n";

        csv_time << N << ","
                 << t_lp_tkr << ","
                 << t_lp_irp << "\n";
    }

    csv.close();
    csv_time.close();

    DBG("LP-only benchmark finished successfully");
    return 0;
}
