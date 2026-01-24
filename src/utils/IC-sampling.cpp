#include "waxman-graph.h"
#include <random>
#include <cmath>
#include <iostream>

using AdjList = std::vector<std::vector<int>>;

AdjList sample_ic_live_edge_adj_list(
    const Graph &G,
    uint64_t seed,
    double INFECT_PROB = 0.2
) {
    const int N = G.nodes.size();

    AdjList sampled_adj(N);

    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uni(0.0, 1.0);

    for (int u = 0; u < N; ++u) {
        for (int v : G.adj_list[u]) {
            if (INFECT_PROB > uni(rng)) {
                sampled_adj[u].push_back(v);
            }
        }
    }
    return sampled_adj;
}

std::vector<AdjList> sample_ic_live_edge_topologies(
    const Graph &G,
    int S,
    uint64_t seed,
    double prob_infect
) {
    std::vector<AdjList> samples;
    samples.reserve(S);

    for (int s = 0; s < S; ++s) {
        samples.push_back(
            sample_ic_live_edge_adj_list(G, seed + s, prob_infect)
        );
    }
    return samples;
}