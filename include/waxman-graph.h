#ifndef WAXMANGRAPH_H
#define WAXMANGRAPH_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include <algorithm>
#include <cstdint> // for uint64_t
#include "Random_number_generator.h" 
#include "SpatialGrid.h"

static const int TIER_VILLAGE = 0;
static const int TIER_CITY   = 1;

struct Center {
    double x, y;
    int tier;
    int id;
};

struct Node {
    int id;
    double x, y;
    int tier;
    int cluster_id;
};


class Graph {
public:

    // --- disable copying ---
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    // --- enable moving ---
    Graph(Graph&& other) noexcept;
    Graph& operator=(Graph&& other) noexcept;

    struct Params {
        int num_cities;
        int num_villages;
        int nodes_per_city;
        int nodes_per_village;
        double alpha, beta, cutoff_prob;
        double world_min_x = 0.0, world_min_y = 0.0, world_max_x = 1.0, world_max_y = 1.0;
        double grid_cell_size = 0.02;
    };

    Params params;
    std::vector<Center> centers;
    std::vector<Node> nodes;
    std::vector<std::unordered_set<int>> adj_list;
    std::vector<double> nodesThreshold;

    SpatialGrid *grid = nullptr;

    Random_number_generator *rng_gen;

    double cutoff_radius;

    Graph(const Params &p, uint64_t seed);
    ~Graph();
    void generate_centers();
    void generate_nodes();
    void build_spatial_index();
    void generate_edges();

    double distance_sq(double x1, double y1, double x2, double y2) const;
    double distance(double x1, double y1, double x2, double y2) const;

    double tier_move_prob(int t) const;
    int pick_target_cluster_weighted(int node_id);

    void remove_edges_of(int u);
    void create_edges_for(int u);
    void move_node(int u, double newx, double newy, int newtier, int new_cluster_id);
    void simulate_movement(int num_steps);

    void print_summary() const;
};

struct UndirectedEdgeRandom {

    uint64_t seed;

    UndirectedEdgeRandom(uint64_t s) : seed(s) {}

    // 64-bit mix function (SplitMix64)
    static uint64_t splitmix64(uint64_t x) {
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }

    // Deterministic uniform [0,1)
    double get(int u, int v, int epoch) const {
        if (u > v) std::swap(u, v);

        uint64_t x = seed;
        x ^= uint64_t(epoch) * 0x9e3779b97f4a7c15ULL;
        x ^= uint64_t(u)     * 0xbf58476d1ce4e5b9ULL;
        x ^= uint64_t(v)     * 0x94d049bb133111ebULL;

        uint64_t h = splitmix64(x);

        // Convert to double in [0,1)
        return (h >> 11) * (1.0 / (1ULL << 53));
    }
};

#endif