// main.cpp
// Dynamic Waxman Graph + Epidemic Diffusion Models (IC & LT)
// Vaccination strategies: Greedy, Local Search, Hill Climbing, PageRank
// Percentage-based infection & vaccination budget
// No snapshot dumping

#include "waxman-graph.h"
#include "Random_number_generator.h"
#include "simulation.h"
#include "strategy.h"

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
    int num_city, num_villages;
    int nodes_per_city, nodes_per_village;
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

    // Defaults
    c.world_min_x = 0;
    c.world_min_y = 0;
    c.world_max_x = 1;
    c.world_max_y = 1;
    c.num_city = 5;
    c.num_villages = 25;
    c.nodes_per_city = 50;
    c.nodes_per_village = 15;
    c.alpha = 0.05;
    c.beta = 0.3;
    c.cutoff_prob = 0.001;
    c.grid_cell_size = 0.15;
    c.num_steps = 50;
    c.initial_infected_percent = 3.0;
    c.vaccination_budget_percent = 5.0;
    c.seed = 2025;
    c.pagerank_alpha = 0.85;
    c.pagerank_tol = 1e-6;

    std::ifstream in(path);
    if (!in) {
        std::cerr << "Config not found. Using defaults.\n";
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
        else if (key == "num_cities") vs >> c.num_city;
        else if (key == "num_villages") vs >> c.num_villages;
        else if (key == "nodes_per_city") vs >> c.nodes_per_city;
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

    // Build graph
    Graph::Params p;
    p.world_min_x = conf.world_min_x;
    p.world_min_y = conf.world_min_y;
    p.world_max_x = conf.world_max_x;
    p.world_max_y = conf.world_max_y;
    p.num_cities = conf.num_city;
    p.num_villages = conf.num_villages;
    p.nodes_per_city = conf.nodes_per_city;
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

    // Percent → absolute counts
    int num_infected = std::max(1, int((conf.initial_infected_percent / 100.0) * N));
    int K = std::max(1, int((conf.vaccination_budget_percent / 100.0) * N));

    std::cout << "Initial infected count = " << num_infected << "\n";
    std::cout << "Vaccination budget K = " << K << "\n";

    // Initial infected
    Random_number_generator rng(conf.seed);
    std::vector<bool> Infected_Node(N, false);
    std::vector<int> infected_list;

    {
        std::vector<int> pool(N);
        for (int i = 0; i < N; i++) pool[i] = i;

        for (int i = 0; i < N; i++) {
            int j = i + int(rng.get_unif() * (N - i));
            if (j >= N) j = N - 1;
            std::swap(pool[i], pool[j]);
        }

        for (int i = 0; i < num_infected; i++) {
            Infected_Node[pool[i]] = true;
            infected_list.push_back(pool[i]);
        }
    }

    // Global vaccination mask
    std::vector<bool> globalVacc(N, true);

    std::filesystem::create_directories("results");

    int stepSize = 0;
    int T = conf.num_steps;

    // Evaluators
    auto ic_eval = [&](Graph &Gcopy,
                       const std::vector<bool> &Vacc,
                       const int &newV,
                       const std::vector<bool> &Inf,
                       const uint64_t &seed,
                       const int &step) {
        return IC_Simulation(Gcopy, Vacc, newV, Inf, seed, step);
    };

    auto lt_eval = [&](Graph &Gcopy,
                       const std::vector<bool> &Vacc,
                       const int &newV,
                       const std::vector<bool> &Inf,
                       const uint64_t &seed,
                       const int &step) {
        return LinearThreshold_Simulator(Gcopy, Vacc, newV, Inf, seed, step);
    };

    std::vector<std::string> models = {"IC", "LT"};
    std::vector<std::string> strategies = {"greedy", "local_search", "hill_climbing", "pagerank"};

    // Run experiments
    for (auto &model : models) {
        std::cout << "===== Running model: " << model << " =====\n";

        std::ofstream summary("results/" + model + "_results.csv");
        summary << "strategy,saved\n";

        for (auto &s : strategies) {
            std::vector<int> vaccinated;

            if (s == "greedy") {
                if (model == "IC")
                    Greedy_Vaccination(g, K, infected_list, vaccinated,
                                       ic_eval, stepSize, T);
                else
                    Greedy_Vaccination(g, K, infected_list, vaccinated,
                                       lt_eval, stepSize, T);
            }
            else if (s == "local_search") {
                if (model == "IC")
                    Local_Search(g, K, infected_list, vaccinated,
                                 ic_eval, stepSize, T, 5000);
                else
                    Local_Search(g, K, infected_list, vaccinated,
                                 lt_eval, stepSize, T, 5000);
            }
            else if (s == "hill_climbing") {
                std::vector<int> seed_vec;
                std::vector<std::pair<int,int>> deg;

                for (int i = 0; i < N; i++)
                    deg.push_back({-(int)g.adj_list[i].size(), i});

                std::sort(deg.begin(), deg.end());
                for (int i = 0; i < K; i++)
                    seed_vec.push_back(deg[i].second);

                if (model == "IC")
                    HIll_Climbing(g, K, infected_list, vaccinated,
                                  ic_eval, stepSize, T, 200, seed_vec);
                else
                    HIll_Climbing(g, K, infected_list, vaccinated,
                                  lt_eval, stepSize, T, 200, seed_vec);
            }
            else if (s == "pagerank") {
                vaccinated = PageRank(g, K, infected_list,
                                      conf.pagerank_alpha,
                                      conf.pagerank_tol, 100);
            }

            // Save vaccinated list
            {
                std::ofstream vf("results/" + model + "_" + s + "_vaccinated.csv");
                for (int v : vaccinated) vf << v << "\n";
            }

            // Evaluate saved nodes
            std::vector<bool> mask = globalVacc;
            for (int v : vaccinated) mask[v] = false;

            int saved = (model == "IC")
                ? IC_Simulation(g, mask, -1, Infected_Node, conf.seed, stepSize)
                : LinearThreshold_Simulator(g, mask, -1, Infected_Node, conf.seed, stepSize);

            summary << s << "," << saved << "\n";
            std::cout << "  Strategy " << s << " saved: " << saved << "\n";
        }

        summary.close();
    }

    std::cout << "All done. Results in ./results\n";
    return 0;
}
