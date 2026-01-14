// ------------------------------------------------------------
// src/benchmark-vaccination.cpp  (DEBUG BUILD)
//
// Average number of nodes saved by different vaccination
// algorithms for Gaussian Waxman graphs
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
    Random_number_generator rng(seed);
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

    DBG("run_baseline: seed=" << seed);

    int N = G.nodes.size();
    std::vector<bool> vaccinated(N, false);
    std::vector<bool> infected(N, false);

    for (int u : initial_infected) {
        assert(u >= 0 && u < N);
        infected[u] = true;
    }

    for (int u : initial_vaccinated) {
        assert(u >= 0 && u < N);
        vaccinated[u] = true;
    }

    int newVaccinatedNode = -1;
    DBG("Calling Simulator (newVaccinatedNode = -1)");
    return G.nodes.size() - Simulator(G, vaccinated, newVaccinatedNode, infected, seed, stepSize);
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

    DBG("run_baseline_with_seeds: seeds=" << seeds.size());

    int total = 0;
    for (uint64_t s : seeds) {
        total += run_baseline(
            G, initial_infected, initial_vaccinated,
            Simulator, s, stepSize
        );
    }
    return total / seeds.size();
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

    DBG("Sampling initial infected: k=" << k << " N=" << N);

    std::set<int> s;
    Random_number_generator rng(seed);

    while ((int)s.size() < k) {
        s.insert(rng.get_int(0, N - 1));
    }
    return std::vector<int>(s.begin(), s.end());
}

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
int main() {

    DBG("Program started");

    Config cfg;
    if (!load_config("config.txt", cfg)) {
        std::cerr << "Failed to load config.txt\n";
        return 1;
    }

    DBG("Config loaded");
    DBG("Diffusion model = " << cfg.diffusion_model);

    SimulatorFn Simulator =
        (cfg.diffusion_model == "LT")
            ? LinearThreshold_Simulator
            : IC_Simulation;

    std::ofstream csv("results/benchmark_results.csv");
    csv << "nodes,baseline,greedy,local_search,hill_climbing,lp_tkr,lp_irp\n";
    csv << std::fixed << std::setprecision(2);

    std::ofstream csv_time("results/benchmark_time.csv");
    csv_time << "nodes,baseline,greedy,local_search,hill_climbing,lp_tkr,lp_irp\n";
    csv_time << std::fixed << std::setprecision(6);


    // --------------------------------------------------------
    for (int N : cfg.node_sizes) {

        double t_baseline = 0.0;
        double t_greedy   = 0.0;
        double t_ls       = 0.0;
        double t_hc       = 0.0;
        double t_lp_tkr   = 0.0;
        double t_lp_irp   = 0.0;


        DBG("======================================");
        DBG("Starting experiment for N = " << N);

        // ---------------- Graph params ----------------
        Graph::Params p;
        p.num_cities = cfg.num_centers;
        p.num_villages = cfg.num_centers * 2;

        p.nodes_per_city =
            static_cast<int>(0.7 * N / cfg.num_centers);

        p.nodes_per_village =
            (N - p.nodes_per_city * cfg.num_centers) / p.num_villages;

        DBG("Graph Params:"
            << " cities=" << p.num_cities
            << " villages=" << p.num_villages
            << " nodes_per_city=" << p.nodes_per_city
            << " nodes_per_village=" << p.nodes_per_village);

        p.alpha = cfg.alpha;
        p.beta = cfg.beta;
        p.cutoff_prob = cfg.cutoff_prob;

        DBG("Constructing Graph...");
        Graph G(p, cfg.seed);
        G.generate_centers();
        G.generate_nodes();
        G.build_spatial_index();
        G.generate_edges();
        DBG("Graph constructed");

        DBG("Graph stats:"
            << " nodes=" << G.nodes.size()
            << " centers=" << G.centers.size()
            << " adj_list=" << G.adj_list.size());

        int k = static_cast<int>(cfg.vaccination_budget_percent * N);
        DBG("Vaccination budget k=" << k);

        auto seeds = generate_seeds(cfg.seed, cfg.T);

        auto initial_infected =
            sample_initial_infected(G, cfg.initial_infected_percent, cfg.seed);

        DBG("Initial infected count=" << initial_infected.size());

        auto t0 = Clock::now();
        int baseline =
            run_baseline_with_seeds(
                G, initial_infected, {},
                Simulator, seeds, cfg.stepSize
            );
        auto t1 = Clock::now();
        t_baseline = elapsed_seconds(t0, t1);
        DBG("Baseline infected=" << baseline);

        // ---------------- Greedy ----------------
        DBG("Running Greedy");
        std::vector<int> greedy;
        t0 = Clock::now();
        Greedy_Vaccination(
            G, k, initial_infected,
            greedy, Simulator,
            cfg.stepSize, cfg.T
        );
        t1 = Clock::now();
        t_greedy = elapsed_seconds(t0, t1);
        DBG("Greedy done");

        int greedy_inf =
            run_baseline_with_seeds(
                G, initial_infected, greedy,
                Simulator, seeds, cfg.stepSize
            );

        // ---------------- Local Search ----------------
        DBG("Running Local Search");
        std::vector<int> ls;
        t0 = Clock::now();
        Local_Search(
            G, k, initial_infected,
            ls, Simulator,
            cfg.stepSize, cfg.T, 10, greedy
        );
        t1 = Clock::now();
        t_ls = elapsed_seconds(t0, t1);
        DBG("Local Search done");


        int ls_inf =
            run_baseline_with_seeds(
                G, initial_infected, ls,
                Simulator, seeds, cfg.stepSize
            );

        // ---------------- Hill Climbing ----------------
        DBG("Running Hill Climbing");
        std::vector<int> hc;
        t0 = Clock::now();
        HIll_Climbing(
            G, k, initial_infected,
            hc, Simulator,
            cfg.stepSize, cfg.T, 10, greedy
        );
        t1 = Clock::now();
        t_hc = elapsed_seconds(t0, t1);
        DBG("Hill Climbing done");


        int hc_inf =
            run_baseline_with_seeds(
                G, initial_infected, hc,
                Simulator, seeds, cfg.stepSize
            );

        // ---------------- LP ----------------
        DBG("Sampling LP live-edge graphs");
        std::vector<Graph> samples =
            (cfg.diffusion_model == "LT")
                ? sample_lt_live_edge_topologies(G, cfg.T, cfg.seed)
                : sample_ic_live_edge_topologies(G, cfg.T, cfg.seed);

        DBG("LP samples generated = " << samples.size());

        DBG("Running LP-TKR");
        t0 = Clock::now();
        auto lp_tkr =
            solve_lp_vaccination(samples, initial_infected, k, RoundingMethod::TKR);
        t1 = Clock::now();
        t_lp_tkr = elapsed_seconds(t0, t1);

        int lp_tkr_inf =
            run_baseline(G, initial_infected, lp_tkr,
                         Simulator, cfg.seed, cfg.stepSize);

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

        DBG("Results for N=" << N
            << ": greedy_saved =" << N - greedy_inf
            << ", ls_saved =" << N - ls_inf
            << ", hc_saved =" << N - hc_inf
            << ", lp_tkr_saved =" << N - lp_tkr_inf
            << ", lp_irp_saved =" << N - lp_irp_inf);
        
        std::cout << "N=" << N
            << " baseline=" << N - baseline
            << " greedy_saved=" << N - greedy_inf
            << " ls_saved=" << N - ls_inf
            << " hc_saved=" << N - hc_inf
            << " lp_tkr_saved=" << N - lp_tkr_inf
            << " lp_irp_saved=" << N - lp_irp_inf
            << std::endl;
            
        csv << N << ","
            << N - baseline << ","
            << N - greedy_inf << ","
            << N - ls_inf << ","
            << N - hc_inf << ","
            << N - lp_tkr_inf << ","
            << N - lp_irp_inf << "\n";

        DBG("Finished N=" << N);

        csv_time << N << ","
         << t_baseline << ","
         << t_greedy   << ","
         << t_ls       << ","
         << t_hc       << ","
         << t_lp_tkr   << ","
         << t_lp_irp   << "\n";
        
    }
    csv.close();
    csv_time.close();
    DBG("Program finished successfully");
    return 0;
}
