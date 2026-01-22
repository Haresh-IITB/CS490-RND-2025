#include "waxman-graph.h"
#include <random>
#include <cmath>
#include <iostream>

Graph sample_ic_live_edge_graph(
    const Graph &G,
    uint64_t seed,
    double INFECT_PROB = 0.2)
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

    for (int u = 0; u < N; u++) {
        for (int v : G.adj_list[u]) {
            if (INFECT_PROB > uni(rng)) {
                Gs.adj_list[u].insert(v);
            }
        }
    }
    return Gs;
}


std::vector<Graph> sample_ic_live_edge_topologies(
    const Graph &G,
    int S,
    uint64_t seed,
    double prob_infect)
{
    std::vector<Graph> samples;
    samples.reserve(S);

    for (int s = 0; s < S; s++) {
        samples.push_back(
            sample_ic_live_edge_graph(G, seed + s)
        );
    }
    return samples;
}
