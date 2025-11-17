#ifndef WAXMANGRAPH_H
#define WAXMANGRAPH_H

#include <vector>
#include <unordered_set>
#include <cstdint> // for uint64_t
#include "Random_number_generator.h" 
#include "SpatialGrid.h"

static const int TIER_VILLAGE = 0;
static const int TIER_TIER3   = 1;
static const int TIER_TIER2   = 2;
static const int TIER_TIER1   = 3;

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

    struct Params {
        int num_t1, num_t2, num_t3, num_villages;
        int nodes_per_t1, nodes_per_t2, nodes_per_t3, nodes_per_village;
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

#endif