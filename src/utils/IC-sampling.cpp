#include "waxman-graph.h"
#include <random>
#include <cmath>
#include <iostream>

double prob_ic_edge(
    const Graph &G,
    int u,
    int v,
    double weight = 2.0) {

    double d = G.distance(
        G.nodes[u].x, G.nodes[u].y,
        G.nodes[v].x, G.nodes[v].y
    );
    double p = std::min((double)1, weight*G.params.alpha * std::exp(-d / G.params.beta));
    return p;
}

Graph sample_ic_live_edge_graph(
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

    for (int u = 0; u < N; u++) {
        for (int v : G.adj_list[u]) {
            double p = prob_ic_edge(G, u, v, 4.0);
            if (p > uni(rng)) {
                Gs.adj_list[u].insert(v);
            }
        }
    }
    return Gs;
}


std::vector<Graph> sample_ic_live_edge_topologies(
    const Graph &G,
    int S,
    uint64_t seed)
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
