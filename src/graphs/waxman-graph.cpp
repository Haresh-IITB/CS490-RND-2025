#include "waxman-graph.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <system_error>


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
    auto add_center = [&](double x, double y, int t){
        Center c; c.x = x; c.y = y; c.tier = t; c.id = (int)centers.size(); centers.push_back(c);
    };

    // Tier‑1
    int n1 = std::max(1, params.num_t1);
    int cols = std::max(1, (int)std::ceil(std::sqrt((double)n1)));
    int rows = std::max(1, (int)std::ceil((double)n1 / cols));
    int placed = 0;

    double cellw = (params.world_max_x - params.world_min_x) / cols;
    double cellh = (params.world_max_y - params.world_min_y) / rows;

    for (int r = 0; r < rows && placed < n1; ++r) {
        for (int c = 0; c < cols && placed < n1; ++c) {
            double cx = params.world_min_x + (c + 0.5) * cellw;
            double cy = params.world_min_y + (r + 0.5) * cellh;

            // Use rng_gen->get_unif() which returns [0, 1]
            double jitter_x = (rng_gen->get_unif() - 0.5) * 0.4 * cellw;
            double jitter_y = (rng_gen->get_unif() - 0.5) * 0.4 * cellh;

            add_center(std::clamp(cx + jitter_x, params.world_min_x, params.world_max_x),
                       std::clamp(cy + jitter_y, params.world_min_y, params.world_max_y),
                       TIER_TIER1);
            placed++;
        }
    }

    // random placement
    auto place_random_with_spacing = [&](int count, double min_dist, int tier) {
        int attempts_limit = 2000;
        while (count > 0 && attempts_limit-- > 0) {
            // Use rng_gen->get_unif() which returns [0, 1]
            double x = rng_gen->get_unif();
            double y = rng_gen->get_unif();

            bool ok = true;
            for (const auto &c : centers) {
                double dx = x - c.x, dy = y - c.y;
                if (std::sqrt(dx*dx + dy*dy) < min_dist) { ok = false; break; }
            }
            if (!ok) continue;

            add_center(x, y, tier);
            count--;
        }
    };

    // -----------------------------
    // Tier‑2: random with 0.12 spacing
    // -----------------------------
    place_random_with_spacing(params.num_t2, 0.12, TIER_TIER2);

    // -----------------------------
    // Tier‑3: random with 0.06 spacing
    // -----------------------------
    place_random_with_spacing(params.num_t3, 0.06, TIER_TIER3);

    // -----------------------------
    // Villages: uniform random (no spacing constraint)
    // -----------------------------
    for (int i = 0; i < params.num_villages; ++i) {
        // Use rng_gen->get_unif() which returns [0, 1]
        double x = rng_gen->get_unif();
        double y = rng_gen->get_unif();
        add_center(x, y, TIER_VILLAGE);
    }
}


void Graph::generate_nodes() {
    nodes.clear();
    int id_counter = 0;
    auto add_cluster_nodes = [&](int count, const Center &c, double spread){
        double variance = spread * spread; 
        for (int i = 0; i < count; ++i) {
            Node n; n.id = id_counter++; 
            // Use rng_gen->get_normal()
            n.x = rng_gen->get_normal(c.x, variance);
            n.y = rng_gen->get_normal(c.y, variance);
            n.tier = c.tier; n.cluster_id = c.id;
            n.x = std::clamp(n.x, params.world_min_x, params.world_max_x);
            n.y = std::clamp(n.y, params.world_min_y, params.world_max_y);
            nodes.push_back(n);
            double theta = rng_gen->get_unif(0.1, 1.0); 
            nodesThreshold.push_back(theta);
        }
    };

    // T1 nodes
    int idx = 0;
    for (int i = 0; i < params.num_t1; ++i) { add_cluster_nodes(params.nodes_per_t1, centers[idx++], 0.02); }
    // T2 nodes
    for (int i = 0; i < params.num_t2; ++i) { add_cluster_nodes(params.nodes_per_t2, centers[idx++], 0.035); }
    // T3 nodes
    for (int i = 0; i < params.num_t3; ++i) { add_cluster_nodes(params.nodes_per_t3, centers[idx++], 0.05); }
    // villages
    for (int i = 0; i < params.num_villages; ++i) { add_cluster_nodes(params.nodes_per_village, centers[idx++], 0.08); }

    adj_list.assign(nodes.size(), std::unordered_set<int>());
}

void Graph::build_spatial_index() {
    if (grid) { delete grid; grid = nullptr; }
    grid = new SpatialGrid(params.world_min_x, params.world_min_y, params.world_max_x, params.world_max_y, params.grid_cell_size);
    for (const auto &n : nodes) grid->insert(n.id, n.x, n.y);
}

void Graph::generate_edges() {
    for (const auto &u : nodes) {
        auto candidates = grid->query_radius(u.x, u.y, cutoff_radius);
        for (int vid : candidates) {
            if (vid == u.id) continue;
            if (vid < u.id) continue; // process each pair once
            const Node &v = nodes[vid];
            double d = distance(u.x, u.y, v.x, v.y);
            double p = params.alpha * std::exp(-d / params.beta);
            if (p <= 0.0) continue;
            // Use rng_gen->get_unif() which returns [0, 1]
            if (rng_gen->get_unif() < p) {
                adj_list[u.id].insert(v.id);
                adj_list[v.id].insert(u.id);
            }
        }
    }
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
double Graph::tier_move_prob(int t) const {
    switch (t) {
        case TIER_VILLAGE: return 0.10;
        case TIER_TIER3:  return 0.05;
        case TIER_TIER2:  return 0.02;
        case TIER_TIER1:  default: return 0.01;
    }
}

int Graph::pick_target_cluster_weighted(int node_id) {
    const Node &u = nodes[node_id];
    std::vector<double> weights; weights.reserve(centers.size());
    double sum = 0.0;
    for (const auto &c : centers) {
        int tier_bias = (c.tier >= u.tier) ? 2 : 1;
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

    double local_move_variance = 0.01 * 0.01;
    double cluster_move_variance = 0.02 * 0.02;

    for (int step = 0; step < num_steps; ++step) {
        std::vector<int> movers; movers.reserve(nodes.size() / 20);
        for (const auto &n : nodes) { 
            double p = tier_move_prob(n.tier); 
            // Use rng_gen->get_unif() which returns [0, 1]
            if (rng_gen->get_unif() < p) movers.push_back(n.id); 
        }
        for (int u : movers) {
            Node &n = nodes[u];
            int target_center_id = pick_target_cluster_weighted(u);
            const Center &tc = centers[target_center_id];
            int newtier = tc.tier;
            double newx, newy;
            if (n.tier == TIER_TIER1 && newtier == TIER_TIER1) {
                // Use rng_gen->get_normal()
                newx = n.x + rng_gen->get_normal(0.0, local_move_variance); 
                newy = n.y + rng_gen->get_normal(0.0, local_move_variance);
            } else {
                newx = rng_gen->get_normal(tc.x, cluster_move_variance); 
                newy = rng_gen->get_normal(tc.y, cluster_move_variance);
            }
            newx = std::clamp(newx, params.world_min_x, params.world_max_x);
            newy = std::clamp(newy, params.world_min_y, params.world_max_y);
            move_node(u, newx, newy, newtier, tc.id);
        }
    }
}


int main() {
    Graph::Params p;
    p.num_t1 = 2; p.num_t2 = 4; p.num_t3 = 6; p.num_villages = 4;
    p.nodes_per_t1 = 200; p.nodes_per_t2 = 10; p.nodes_per_t3 = 4; p.nodes_per_village = 2;
    p.alpha = 0.4; p.beta = 0.1; p.cutoff_prob = 1e-3; p.grid_cell_size = 0.02;

    const int num_steps = 100;
    const std::string out_dir = "snapshots";

    Graph g(p, 2025);
    g.generate_centers();
    g.generate_nodes();
    g.build_spatial_index();
    g.generate_edges();
    g.print_summary();

    std::error_code ec;
    std::filesystem::create_directories(out_dir, ec);
    if (ec) { std::cerr << "Failed to create output directory: " << ec.message() << '\n'; return 1; }

    auto dump_frame = [&](int step) {
        char node_filename[256]; char edge_filename[256];
        std::snprintf(node_filename, sizeof(node_filename), "%s/snapshot_%04d_nodes.csv", out_dir.c_str(), step);
        std::snprintf(edge_filename, sizeof(edge_filename), "%s/snapshot_%04d_edges.csv", out_dir.c_str(), step);
        std::ofstream nf(node_filename);
        nf << "id,x,y,tier,cluster\n";
        for (const auto &n : g.nodes) nf << n.id << "," << n.x << "," << n.y << "," << n.tier << "," << n.cluster_id << '\n';
        nf.close();
        std::ofstream ef(edge_filename);
        ef << "u,v\n";
        for (int u = 0; u < (int)g.adj_list.size(); ++u) for (int v : g.adj_list[u]) if (u < v) ef << u << "," << v << '\n';
        ef.close();
    };

    dump_frame(0);
    for (int step = 1; step <= num_steps; ++step) {
        g.simulate_movement(1);
        dump_frame(step);
        if (step % 10 == 0) std::cout << "Completed step " << step << "\n";
    }
    g.print_summary();
    std::cout << "Snapshots written to " << out_dir << " frames 0.." << num_steps << "\n";
    return 0;
}