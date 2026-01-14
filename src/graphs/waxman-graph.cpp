#include "waxman-graph.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <system_error>

// 1. Reduce attempts limit 
// 2. Make the min distance while placing a parameter (dynamic)
// 3. Make variance of the t1, t2, and t3 citites a dynamic parameter, Use non-normal distribution 
// 4. The center picked is prop to the tier bias ** -> HYPERPARAMETER

#define DEBUG_MOVE 0

Graph::Graph(Graph&& other) noexcept {
    *this = std::move(other);
}

Graph& Graph::operator=(Graph&& other) noexcept {
    if (this != &other) {
        // steal resources
        grid = other.grid;
        rng_gen = other.rng_gen;

        other.grid = nullptr;
        other.rng_gen = nullptr;

        centers = std::move(other.centers);
        nodes = std::move(other.nodes);
        adj_list = std::move(other.adj_list);
        nodesThreshold = std::move(other.nodesThreshold);
        params = other.params;
        cutoff_radius = other.cutoff_radius;
    }
    return *this;
}

Graph::Graph(const Params &p, uint64_t seed)
    : params(p) {
    
    rng_gen = new Random_number_generator(seed);
    
    double safe_alpha = std::max(1e-9, params.alpha);
    cutoff_radius = -params.beta * std::log(params.cutoff_prob / safe_alpha);
    double world_diag = std::hypot(params.world_max_x - params.world_min_x,
                                   params.world_max_y - params.world_min_y); // (Diagonal distance = sqrt(w^2 + h^2))
    if (cutoff_radius > world_diag) cutoff_radius = world_diag;
}

Graph::~Graph() {
    if (grid) delete grid; grid = nullptr;
    // Clean up the random number generator
    if (rng_gen) delete rng_gen; rng_gen = nullptr;
}

void Graph::generate_centers() {
    centers.clear();

    auto add_center = [&](double x, double y, int tier) {
        Center c;
        c.x = x; c.y = y;
        c.tier = tier;
        c.id = (int)centers.size();
        centers.push_back(c);
    };

    // -------- Cities (grid-based placement) --------
    int n = std::max(1, params.num_cities);
    int cols = std::ceil(std::sqrt(n));
    int rows = std::ceil((double)n / cols);

    double cellw = (params.world_max_x - params.world_min_x) / cols;
    double cellh = (params.world_max_y - params.world_min_y) / rows;

    int placed = 0;
    for (int r = 0; r < rows && placed < n; ++r) {
        for (int c = 0; c < cols && placed < n; ++c) {
            double cx = params.world_min_x + (c + 0.5) * cellw;
            double cy = params.world_min_y + (r + 0.5) * cellh;

            double jx = (rng_gen->get_unif() - 0.5) * 0.3 * cellw;
            double jy = (rng_gen->get_unif() - 0.5) * 0.3 * cellh;

            add_center(
                std::clamp(cx + jx, params.world_min_x, params.world_max_x),
                std::clamp(cy + jy, params.world_min_y, params.world_max_y),
                TIER_CITY
            );
            placed++;
        }
    }

    // -------- Villages (uniform random) --------
    for (int i = 0; i < params.num_villages; ++i) {
        add_center(
            rng_gen->get_unif(),
            rng_gen->get_unif(),
            TIER_VILLAGE
        );
    }
}


void Graph::generate_nodes() {
    nodes.clear();
    nodesThreshold.clear();
    int id_counter = 0;

    auto add_cluster_nodes = [&](int count, const Center &c, double spread) {
        double var = spread * spread;
        for (int i = 0; i < count; ++i) {
            Node n;
            n.id = id_counter++;
            n.x = rng_gen->get_normal(c.x, var);
            n.y = rng_gen->get_normal(c.y, var);
            n.tier = c.tier;
            n.cluster_id = c.id;

            n.x = std::clamp(n.x, params.world_min_x, params.world_max_x);
            n.y = std::clamp(n.y, params.world_min_y, params.world_max_y);

            nodes.push_back(n);
            nodesThreshold.push_back(rng_gen->get_unif(0.1, 1.0));
        }
    };

    int idx = 0;

    // Cities
    for (int i = 0; i < params.num_cities; ++i) {
        add_cluster_nodes(params.nodes_per_city, centers[idx++], 0.03);
    }

    // Villages
    for (int i = 0; i < params.num_villages; ++i) {
        add_cluster_nodes(params.nodes_per_village, centers[idx++], 0.07);
    }

    adj_list.assign(nodes.size(), std::unordered_set<int>());
}


void Graph::build_spatial_index() {
    if (grid) { delete grid; grid = nullptr; }
    grid = new SpatialGrid(params.world_min_x, params.world_min_y, params.world_max_x, params.world_max_y, params.grid_cell_size);
    for (const auto &n : nodes) grid->insert(n.id, n.x, n.y);
}

void Graph::generate_edges() {
    int edgeCnt = 0 ; 
    for (const auto &u : nodes) {
        auto candidates = grid->query_radius(u.x, u.y, cutoff_radius); // Costly operation 
        for (int vid : candidates) {
            if (vid <= u.id) continue; // process each pair once
            const Node &v = nodes[vid];
            double d = distance(u.x, u.y, v.x, v.y);
            double p = params.alpha * std::exp(-d / params.beta);
            if (p <= 0.0) continue;
            // Use rng_gen->get_unif() which returns [0, 1]
            if (rng_gen->get_unif() < p) {
                adj_list[u.id].insert(v.id);
                adj_list[v.id].insert(u.id);
                edgeCnt++ ;
            }
        }
    }

    std::cout << "Generated edges: " << edgeCnt << "\n" ;
}


double Graph::distance_sq(double x1, double y1, double x2, double y2) const { double dx = x1 - x2; double dy = y1 - y2; return dx*dx + dy*dy; }
double Graph::distance(double x1, double y1, double x2, double y2) const { return std::sqrt(distance_sq(x1,y1,x2,y2)); }

void Graph::print_summary() const {
    std::cout << "Graph summary: V=" << nodes.size() << " centers=" << centers.size() << "\n";
    long long totalE = 0;
    for (const auto &s : adj_list) totalE += s.size();
    std::cout << "Edges (undirected counted twice)=" << totalE << ", unique edges ~" << (totalE/2) << "\n";
}

// movement
// This denotes the probability that a person from a specific tier decides to moves 
// That is Village person has a probability of 0.10 to move from village 
double Graph::tier_move_prob(int t) const {
    if (t == TIER_VILLAGE) return 0.10;
    return 0.03; // CITY
}

// Now pick a center at random for the movement
// The center picked is prop to 1/dist 
// The center picked is prop to the tier bias ** -> HYPERPARAMETER
int Graph::pick_target_cluster_weighted(int node_id) {
    const Node &u = nodes[node_id];
    std::vector<double> weights; weights.reserve(centers.size());
    double sum = 0.0;
    for (const auto &c : centers) {
        int tier_bias = (c.tier == TIER_CITY) ? 2 : 1;
        double d2 = distance_sq(u.x, u.y, c.x, c.y) + 1e-6;
        double w = tier_bias * (1.0 / d2);
        weights.push_back(w);
        sum += w;
    }
    if (sum <= 0.0) return u.cluster_id;
    // Removed: std::uniform_real_distribution<double> d01(0.0, sum);
    // Use rng_gen->get_unif(min, max)
    double r = rng_gen->get_unif(0.0, sum);
    double acc = 0.0;
    for (size_t i = 0; i < centers.size(); ++i) { acc += weights[i]; if (r <= acc) return (int)centers[i].id; }
    return centers.back().id;
}

void Graph::remove_edges_of(int u) {
    for (int v : adj_list[u]) adj_list[v].erase(u);
    adj_list[u].clear();
}

void Graph::create_edges_for(int u) {
    const Node &nu = nodes[u];
    auto candidates = grid->query_radius(nu.x, nu.y, cutoff_radius);
    for (int vid : candidates) {
        if (vid == u) continue;
        if (adj_list[u].count(vid)) continue;
        double d = distance(nu.x, nu.y, nodes[vid].x, nodes[vid].y);
        double p = params.alpha * std::exp(-d / params.beta);
        // Use rng_gen->get_unif() which returns [0, 1]
        if (rng_gen->get_unif() < p) {
            adj_list[u].insert(vid);
            adj_list[vid].insert(u);
        }
    }
}

void Graph::move_node(int u, double newx, double newy, int newtier, int new_cluster_id) {
    double oldx = nodes[u].x, oldy = nodes[u].y;
    remove_edges_of(u);
    grid->update(u, oldx, oldy, newx, newy);
    nodes[u].x = newx; nodes[u].y = newy; nodes[u].tier = newtier; nodes[u].cluster_id = new_cluster_id;
    create_edges_for(u);
}

void Graph::simulate_movement(int num_steps) {

    double local_move_variance   = 0.01 * 0.01;
    double cluster_move_variance = 0.02 * 0.02;

#if DEBUG_MOVE
    std::cout << "\n===== SIMULATE MOVEMENT START =====\n";
    std::cout << "num_steps = " << num_steps
              << ", local_var = " << local_move_variance
              << ", cluster_var = " << cluster_move_variance << "\n";
#endif

    for (int step = 0; step < num_steps; ++step) {

#if DEBUG_MOVE
        std::cout << "\n--- Movement step " << step << " ---\n";
#endif

        std::vector<int> movers;
        movers.reserve(nodes.size() / 20);

        // Decide movers
        for (const auto &n : nodes) {
            double p = tier_move_prob(n.tier);
            double r = rng_gen->get_unif();

#if DEBUG_MOVE
            std::cout << "Node " << n.id
                      << " [tier=" << n.tier << "] "
                      << "move_prob=" << p
                      << " rand=" << r;
#endif

            if (r < p) {
                movers.push_back(n.id);
#if DEBUG_MOVE
                std::cout << "  -> MOVES\n";
#endif
            } else {
#if DEBUG_MOVE
                std::cout << "  -> stays\n";
#endif
            }
        }

#if DEBUG_MOVE
        std::cout << "Total movers this step: " << movers.size() << "\n";
#endif

        // Execute moves
        for (int u : movers) {
            Node &n = nodes[u];
            int old_cluster = n.cluster_id;
            int old_tier = n.tier;
            double oldx = n.x, oldy = n.y;

            int target_center_id = pick_target_cluster_weighted(u);
            const Center &tc = centers[target_center_id];
            int newtier = tc.tier;

            double newx, newy;

            if (n.tier == TIER_CITY && newtier == TIER_CITY) {
                newx = n.x + rng_gen->get_normal(0.0, local_move_variance);
                newy = n.y + rng_gen->get_normal(0.0, local_move_variance);
#if DEBUG_MOVE
                std::cout << "Node " << u
                          << " CITY->CITY local move\n";
#endif
            } else {
                newx = rng_gen->get_normal(tc.x, cluster_move_variance);
                newy = rng_gen->get_normal(tc.y, cluster_move_variance);
#if DEBUG_MOVE
                std::cout << "Node " << u
                          << " cluster move to center "
                          << target_center_id << "\n";
#endif
            }

            newx = std::clamp(newx, params.world_min_x, params.world_max_x);
            newy = std::clamp(newy, params.world_min_y, params.world_max_y);

#if DEBUG_MOVE
            std::cout << std::fixed << std::setprecision(3)
                      << "Node " << u
                      << ": (" << oldx << "," << oldy << ")"
                      << " -> (" << newx << "," << newy << ")"
                      << " tier " << old_tier << " -> " << newtier
                      << " cluster " << old_cluster << " -> " << tc.id
                      << "\n";
#endif

            move_node(u, newx, newy, newtier, tc.id);
        }
    }

#if DEBUG_MOVE
    std::cout << "===== SIMULATE MOVEMENT END =====\n";
#endif
}

// Main function for testing
// int main() {
//     Graph::Params p;
//     p.num_t1 = 2; p.num_t2 = 4; p.num_t3 = 6; p.num_villages = 4;
//     p.nodes_per_t1 = 200; p.nodes_per_t2 = 10; p.nodes_per_t3 = 4; p.nodes_per_village = 2;
//     p.alpha = 0.4; p.beta = 0.1; p.cutoff_prob = 1e-3; p.grid_cell_size = 0.02;

//     const int num_steps = 100;
//     const std::string out_dir = "snapshots";

//     Graph g(p, 2025);
//     g.generate_centers();
//     g.generate_nodes();
//     g.build_spatial_index();
//     g.generate_edges();
//     g.print_summary();

//     std::error_code ec;
//     std::filesystem::create_directories(out_dir, ec);
//     if (ec) { std::cerr << "Failed to create output directory: " << ec.message() << '\n'; return 1; }

//     auto dump_frame = [&](int step) {
//         char node_filename[256]; char edge_filename[256];
//         std::snprintf(node_filename, sizeof(node_filename), "%s/snapshot_%04d_nodes.csv", out_dir.c_str(), step);
//         std::snprintf(edge_filename, sizeof(edge_filename), "%s/snapshot_%04d_edges.csv", out_dir.c_str(), step);
//         std::ofstream nf(node_filename);
//         nf << "id,x,y,tier,cluster\n";
//         for (const auto &n : g.nodes) nf << n.id << "," << n.x << "," << n.y << "," << n.tier << "," << n.cluster_id << '\n';
//         nf.close();
//         std::ofstream ef(edge_filename);
//         ef << "u,v\n";
//         for (int u = 0; u < (int)g.adj_list.size(); ++u) for (int v : g.adj_list[u]) if (u < v) ef << u << "," << v << '\n';
//         ef.close();
//     };

//     dump_frame(0);
//     for (int step = 1; step <= num_steps; ++step) {
//         g.simulate_movement(1);
//         dump_frame(step);
//         if (step % 10 == 0) std::cout << "Completed step " << step << "\n";
//     }
//     g.print_summary();
//     std::cout << "Snapshots written to " << out_dir << " frames 0.." << num_steps << "\n";
//     return 0;
// }