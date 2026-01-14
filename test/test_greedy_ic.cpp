#include <iostream>
#include <vector>
#include <functional>
#include <iomanip>

#include "waxman-graph.h"

#include <fstream>

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

    // ---- Nodes ----
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

    // ---- Edges ----
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


// ---- Forward declarations ----
int IC_Simulation(
    Graph &G,
    const std::vector<bool> &Vaccinated_Node,
    const int &newVaccinatedNode,
    const std::vector<bool> &Infected_Node,
    const uint64_t &seed,
    const int &stepSize
);

int LinearThreshold_Simulator(
    Graph & G,
    const std::vector<bool> & Vaccinated_Node,
    const int & newVaccinatedNode, 
    const std::vector<bool> & Infected_Node,
    const uint64_t & seed,
    const int & stepSize
);

void Greedy_Vaccination(
    Graph &G,
    const int & k,
    const std::vector<int> &initial_infected,
    std::vector<int> &out_vaccinated_nodes,
    std::function<int(
        Graph &,
        const std::vector<bool> &,
        const int &,
        const std::vector<bool> &,
        const uint64_t &,
        const int &
    )> Simulator,
    int stepSize,
    int T
);

// ---- Pretty printing helpers ----

std::string tier_name(int t) {
    return (t == TIER_CITY) ? "CITY" : "VILLAGE";
}

void print_centers(const Graph &G) {
    std::cout << "\n=== CENTERS ===\n";
    for (const auto &c : G.centers) {
        std::cout << "Center " << c.id
                  << " [" << tier_name(c.tier) << "] "
                  << " (" << c.x << ", " << c.y << ")\n";
    }
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

void print_adjacency(const Graph &G) {
    std::cout << "\n=== ADJACENCY LIST ===\n";
    for (size_t u = 0; u < G.adj_list.size(); ++u) {
        std::cout << u << " -> ";
        for (int v : G.adj_list[u])
            std::cout << v << " ";
        std::cout << "\n";
    }
}

void print_mask(const std::string &name, const std::vector<bool> &m) {
    std::cout << name << ": ";
    for (bool b : m) std::cout << (b ? "1 " : "0 ");
    std::cout << "\n";
}

// --------------------------------

int main() {
    std::cout << "\n========== GREEDY + IC DEBUG TEST (2-TIER) ==========\n";

    // 1 Build a VERY SMALL 2-tier graph
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
    print_centers(G);
    print_nodes(G);
    print_adjacency(G);

    // 2 Initial infection
    std::vector<int> initial_infected = {0};

    std::cout << "\nInitial infected nodes: ";
    for (int u : initial_infected) std::cout << u << " ";
    std::cout << "\n";

    std::vector<bool> infected_mask(G.nodes.size(), false);
    infected_mask[0] = true;

    std::vector<bool> no_vacc(G.nodes.size(), false);

    // 3 IC without vaccination
    int saved0 = LinearThreshold_Simulator(
        G,
        no_vacc,
        -1,
        infected_mask,
        999,
        1
    );

    std::cout << "\nSaved nodes WITHOUT vaccination: "
              << saved0 << " / " << G.nodes.size() << "\n";

    // 4️ Greedy vaccination
    std::vector<int> vaccinated;

    Greedy_Vaccination(
        G,
        2,              // k
        initial_infected,
        vaccinated,
        LinearThreshold_Simulator,
        1,              // stepSize
        3               // T (small for clarity)
    );

    // 5 Final result
    std::cout << "\n=== FINAL VACCINATED SET ===\n";
    for (int u : vaccinated)
        std::cout << "Vaccinated node: " << u << "\n";

    std::cout << "\n========== TEST COMPLETE ==========\n";

    dump_graph_csv(
    G,
    "results/debug_nodes.csv",
    "results/debug_edges.csv",
    vaccinated,
    initial_infected
);

std::cout << "\nGraph written to:\n"
          << "  results/debug_nodes.csv\n"
          << "  results/debug_edges.csv\n";

    return 0;
}
