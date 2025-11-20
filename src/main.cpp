// main.cpp
// Dynamic Waxman Graph + Epidemic Diffusion Models (IC & LT)
// Vaccination strategies: greedy, local_search, hill_climbing, PageRank
// Percentage-based infection & vaccination k
// No snapshot dumping

#include "waxman-graph.h"
#include "Random_number_generator.h"
#include "simulation.h"   // IC_Simulation, LT_Simulation
#include "strategy.h"     // greedy_vaccination, Local_search, hill_climbing, PageRank

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>

// ---------------- config parsing ----------------
struct Config {
    double world_min_x, world_min_y, world_max_x, world_max_y;
    int num_t1, num_t2, num_t3, num_villages;
    int nodes_per_t1, nodes_per_t2, nodes_per_t3, nodes_per_village;
    double alpha, beta, cutoff_prob, grid_cell_size;
    int num_steps;
    double initial_infected_percent;
    double vaccination_budget_percent;
    uint64_t seed;
    double pagerank_alpha, pagerank_tol;
};

static std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, b - a + 1);
}

Config read_config(const std::string &path) {
    Config c{};

    // defaults
    c.world_min_x = 0;
    c.world_min_y = 0;
    c.world_max_x = 10;
    c.world_max_y = 10;
    c.num_t1 = 4; c.num_t2 = 8; c.num_t3 = 16; c.num_villages = 40;
    c.nodes_per_t1 = 200; c.nodes_per_t2 = 100; c.nodes_per_t3 = 40; c.nodes_per_village = 12;
    c.alpha = 0.4; c.beta = 0.1; c.cutoff_prob = 0.001; c.grid_cell_size = 0.15;
    c.num_steps = 50;
    c.initial_infected_percent = 1.0;
    c.vaccination_budget_percent = 5.0;
    c.seed = 2025;
    c.pagerank_alpha = 0.85;
    c.pagerank_tol = 1e-6;

    std::ifstream in(path);
    if (!in) {
        std::cerr << "Config not found. Using defaults\n";
        return c;
    }

    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));
        std::istringstream vs(val);

        if (key == "world_min_x") vs >> c.world_min_x;
        else if (key == "world_min_y") vs >> c.world_min_y;
        else if (key == "world_max_x") vs >> c.world_max_x;
        else if (key == "world_max_y") vs >> c.world_max_y;
        else if (key == "num_t1") vs >> c.num_t1;
        else if (key == "num_t2") vs >> c.num_t2;
        else if (key == "num_t3") vs >> c.num_t3;
        else if (key == "num_villages") vs >> c.num_villages;
        else if (key == "nodes_per_t1") vs >> c.nodes_per_t1;
        else if (key == "nodes_per_t2") vs >> c.nodes_per_t2;
        else if (key == "nodes_per_t3") vs >> c.nodes_per_t3;
        else if (key == "nodes_per_village") vs >> c.nodes_per_village;
        else if (key == "alpha") vs >> c.alpha;
        else if (key == "beta") vs >> c.beta;
        else if (key == "cutoff_prob") vs >> c.cutoff_prob;
        else if (key == "grid_cell_size") vs >> c.grid_cell_size;
        else if (key == "num_steps") vs >> c.num_steps;
        else if (key == "initial_infected_percent") vs >> c.initial_infected_percent;
        else if (key == "vaccination_budget_percent") vs >> c.vaccination_budget_percent;
        else if (key == "seed") vs >> c.seed;
        else if (key == "pagerank_alpha") vs >> c.pagerank_alpha;
        else if (key == "pagerank_tol") vs >> c.pagerank_tol;
    }
    return c;
}


// ---------------- main ----------------
int main(int argc, char **argv) {
    std::string cfg = "config.txt";
    if (argc >= 2) cfg = argv[1];

    Config conf = read_config(cfg);

    // Build graph parameters
    Graph::Params p;
    p.world_min_x = conf.world_min_x;
    p.world_min_y = conf.world_min_y;
    p.world_max_x = conf.world_max_x;
    p.world_max_y = conf.world_max_y;
    p.num_t1 = conf.num_t1;
    p.num_t2 = conf.num_t2;
    p.num_t3 = conf.num_t3;
    p.num_villages = conf.num_villages;
    p.nodes_per_t1 = conf.nodes_per_t1;
    p.nodes_per_t2 = conf.nodes_per_t2;
    p.nodes_per_t3 = conf.nodes_per_t3;
    p.nodes_per_village = conf.nodes_per_village;
    p.alpha = conf.alpha;
    p.beta = conf.beta;
    p.cutoff_prob = conf.cutoff_prob;
    p.grid_cell_size = conf.grid_cell_size;

    Graph g(p, conf.seed);

    g.generate_centers();
    g.generate_nodes();
    g.build_spatial_index();
    g.generate_edges();
    g.print_summary();

    int N = g.nodes.size();
    std::cout << "Total nodes = " << N << "\n";

    // Convert percentage → absolute count
    int num_infected = std::max(1, int((conf.initial_infected_percent / 100.0) * N));
    int K = std::max(1, int((conf.vaccination_budget_percent / 100.0) * N));

    std::cout << "Initial infected count = " << num_infected << "\n";
    std::cout << "Vaccination budget K = " << K << "\n";

    // Random initial infected
    Random_number_generator rng(conf.seed);
    std::vector<int> initial_infected;
    {
        std::vector<int> pool(N);
        for (int i = 0; i < N; i++) pool[i] = i;
        for (int i = 0; i < N; i++) {
            int j = i + int(rng.get_unif() * (N - i));
            if (j >= N) j = N - 1;
            std::swap(pool[i], pool[j]);
        }
        initial_infected.assign(pool.begin(), pool.begin() + num_infected);
    }

    // Vaccinable mask
    std::vector<bool> globalVacc(N, true);
    // for (int v : initial_infected) globalVacc[v] = false;

    std::filesystem::create_directories("results");

    // Two models
    std::vector<std::string> models = {"IC", "LT"};
    // Strategies
    std::vector<std::string> strategies = {"greedy", "local_search", "hill_climbing", "pagerank"};

    // Evaluators
    auto ic_eval = [&](Graph &Gcopy, std::vector<bool> &mask, const int &c, const std::vector<int> &I) {
        return IC_Simulation(Gcopy, mask, c, I);
    };
    auto lt_eval = [&](Graph &Gcopy, std::vector<bool> &mask, const int &c, const std::vector<int> &I) {
        return LT_Simulation(Gcopy, mask, c, I);
    };

    // Run everything
    for (auto &model : models) {
        std::cout << "===== Running model: " << model << " =====\n";

        std::ofstream summary("results/" + model + "_results.csv");
        summary << "strategy,saved\n";

        for (auto &s : strategies) {
            std::vector<int> vaccinated;

            if (s == "greedy") {
                if (model == "IC") vaccinated = greedy_vaccination(g, K, initial_infected, ic_eval);
                else vaccinated = greedy_vaccination(g, K, initial_infected, lt_eval);
            }
            else if (s == "local_search") {
                if (model == "IC") vaccinated = Local_search(g, K, initial_infected, ic_eval, 5000);
                else vaccinated = Local_search(g, K, initial_infected, lt_eval, 5000);
            }
            else if (s == "hill_climbing") {
                // initial seed = degree-top-K
                std::vector<int> seed_vec;
                {
                    std::vector<std::pair<int,int>> deg;
                    for (int i = 0; i < N; i++) deg.push_back({-(int)g.adj_list[i].size(), i});
                    std::sort(deg.begin(), deg.end());
                    for (int i = 0; i < K; i++) seed_vec.push_back(deg[i].second);
                }
                if (model == "IC") vaccinated = hill_climbing(g, K, initial_infected, ic_eval, seed_vec, 200);
                else vaccinated = hill_climbing(g, K, initial_infected, lt_eval, seed_vec, 200);
            }
            else if (s == "pagerank") {
                vaccinated = PageRank(g, K, initial_infected, conf.pagerank_alpha, conf.pagerank_tol, 100);
            }

            // write list
            {
                std::ofstream vf("results/" + model + "_" + s + "_vaccinated.csv");
                for (int v : vaccinated) vf << v << "\n";
            }

            // Evaluate final saved nodes
            std::vector<bool> mask = globalVacc;
            for (int v : vaccinated) mask[v] = false;

            int saved = (model == "IC")
                        ? IC_Simulation(g, mask, -1, initial_infected)
                        : LT_Simulation(g, mask, -1, initial_infected);

            summary << s << "," << saved << "\n";
            std::cout << "  Strategy " << s << " saved: " << saved << "\n";
        }

        summary.close();
    }

    std::cout << "All done. Results in ./results\n";
    return 0;
}
