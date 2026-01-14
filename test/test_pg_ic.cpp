#include <iostream>
#include <vector>
#include <functional>
#include <iomanip>
#include <fstream>
#include <numeric>

#include "waxman-graph.h"

// ---------------- CSV DUMP ----------------

void dump_graph_csv(const Graph &G,
                    const std::string &node_file,
                    const std::string &edge_file,
                    const std::vector<int> &vaccinated,
                    const std::vector<int> &initial_infected)
{
    std::vector<int> is_vacc(G.nodes.size(), 0);
    std::vector<int> is_inf(G.nodes.size(), 0);

    for (int u : vaccinated) is_vacc[u] = 1;
    for (int u : initial_infected) is_inf[u] = 1;

    std::ofstream nf(node_file);
    nf << "id,x,y,tier,cluster,vaccinated,infected\n";
    for (const auto &n : G.nodes) {
        nf << n.id << ","
           << n.x << ","
           << n.y << ","
           << n.tier << ","
           << n.cluster_id << ","
           << is_vacc[n.id] << ","
           << is_inf[n.id] << "\n";
    }
    nf.close();

    std::ofstream ef(edge_file);
    ef << "u,v\n";
    for (size_t u = 0; u < G.adj_list.size(); ++u) {
        for (int v : G.adj_list[u]) {
            if (u < (size_t)v)
                ef << u << "," << v << "\n";
        }
    }
    ef.close();
}

// ---------------- FORWARD DECLS ----------------

int LinearThreshold_Simulator(
    Graph & G,
    const std::vector<bool> & Vaccinated_Node,
    const int & newVaccinatedNode,
    const std::vector<bool> & Infected_Node,
    const uint64_t & seed,
    const int & stepSize
);

std::vector<int> PageRank(
    Graph & G,
    const int & K,
    const std::vector<int> & InfectedNodes,
    const double & alpha,
    const double & tolerance,
    int max_iter,
    bool method
);

// ---------------- PRETTY PRINT ----------------

std::string tier_name(int t) {
    return (t == TIER_CITY) ? "CITY" : "VILLAGE";
}

void print_nodes(const Graph &G) {
    std::cout << "\n=== NODES ===\n";
    for (const auto &n : G.nodes) {
        std::cout << "Node " << n.id
                  << " [" << tier_name(n.tier) << "] "
                  << " cluster=" << n.cluster_id
                  << " (" << std::fixed << std::setprecision(3)
                  << n.x << ", " << n.y << ")\n";
    }
}

// ---------------- MAIN ----------------

int main() {
    std::cout << "\n========== PAGERANK + IC DEBUG TEST ==========\n";

    // 1) Small graph
    Graph::Params p;
    p.num_cities = 2;
    p.num_villages = 2;
    p.nodes_per_city = 4;
    p.nodes_per_village = 2;

    p.alpha = 0.8;
    p.beta = 0.25;
    p.cutoff_prob = 1e-3;
    p.grid_cell_size = 0.05;

    uint64_t seed = 123;
    Graph G(p, seed);

    G.generate_centers();
    G.generate_nodes();
    G.build_spatial_index();
    G.generate_edges();

    G.print_summary();
    print_nodes(G);

    // 2) Initial infection
    std::vector<int> initial_infected = {0};

    std::vector<bool> infected_mask(G.nodes.size(), false);
    infected_mask[0] = true;

    std::vector<bool> no_vacc(G.nodes.size(), false);

    // 3) Baseline LT without vaccination
    int saved0 = LinearThreshold_Simulator(
        G,
        no_vacc,
        -1,
        infected_mask,
        999,
        1
    );

    std::cout << "\nSaved WITHOUT vaccination: "
              << saved0 << " / " << G.nodes.size() << "\n";

    // 4) PageRank-based vaccination
    std::vector<int> vaccinated = PageRank(
        G,
        2,                  // K
        initial_infected,
        0.85,               // PageRank alpha
        1e-6,               // tolerance
        10000,                // max_iter
        false                // method (neighbor-guarding)
    );

    std::cout << "\n=== PAGERANK VACCINATED SET ===\n";
    for (int u : vaccinated)
        std::cout << "Vaccinated node: " << u << "\n";

    // Build vaccination mask
    std::vector<bool> vacc_mask(G.nodes.size(), false);
    for (int u : vaccinated)
        vacc_mask[u] = true;

    // 5) LT with PageRank vaccination
    int saved_pg = LinearThreshold_Simulator(
        G,
        vacc_mask,
        -1,
        infected_mask,
        999,
        1
    );

    std::cout << "\nSaved WITH PageRank vaccination: "
              << saved_pg << " / " << G.nodes.size() << "\n";

    // 6) Dump CSV
    dump_graph_csv(
        G,
        "results/pg_debug_nodes.csv",
        "results/pg_debug_edges.csv",
        vaccinated,
        initial_infected
    );

    std::cout << "\nGraph written to:\n"
              << "  results/pg_debug_nodes.csv\n"
              << "  results/pg_debug_edges.csv\n";

    std::cout << "\n========== TEST COMPLETE ==========\n";

    return 0;
}
