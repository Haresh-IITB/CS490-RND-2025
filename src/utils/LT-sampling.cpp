#include "waxman-graph.h"
#include <vector>
#include <random>

Graph sample_lt_live_edge_graph(
    const Graph &G,
    uint64_t seed)
{
    const int N = G.nodes.size();

    Graph::Params params_copy = G.params;
    Graph Gs(params_copy, seed);

    Gs.centers = G.centers;
    Gs.nodes = G.nodes;
    Gs.adj_list = G.adj_list;
    Gs.nodesThreshold = G.nodesThreshold;
    Gs.build_spatial_index();
    
    Gs.adj_list.clear();
    Gs.adj_list.resize(N);

    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uni(0.0, 1.0);


    for (int v = 0; v < N; v++) {
        std::vector<int> in_neighbors;
        std::vector<double> weights;

        double weight_sum = 0.0;
        for (int u : G.adj_list[v]) {
            double d = G.distance(
                    G.nodes[u].x, G.nodes[u].y,
                    G.nodes[v].x, G.nodes[v].y
            );
            double p = G.params.alpha * std::exp(-d / G.params.beta);
            if (p > 0) {
                in_neighbors.push_back(u);
                weights.push_back(p);
                weight_sum += p;
            }
        }

        if (in_neighbors.empty())
            continue;

        // Normalize weights so sum <= 1
        for (double &w : weights)
            w /= weight_sum;

        double r = uni(rng);
        double cumulative = 0.0;

        for (size_t i = 0; i < in_neighbors.size(); i++) {
            cumulative += weights[i];
            if (r <= cumulative) {
                int u = in_neighbors[i];
                Gs.adj_list[u].insert(v); 
                break;
            }
        }
    }

    return Gs;
}


std::vector<Graph> sample_lt_live_edge_topologies(
    const Graph &G,
    int S,
    uint64_t seed)
{
    std::vector<Graph> samples;
    samples.reserve(S);

    for (int s = 0; s < S; s++) {
        samples.push_back(
            sample_lt_live_edge_graph(G, seed + s)
        );
    }

    return samples;
}
