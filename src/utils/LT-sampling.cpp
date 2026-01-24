#include "waxman-graph.h"
#include <vector>
#include <random>
#include <cmath>
#include <iterator>

using AdjList = std::vector<std::vector<int>>;

AdjList sample_lt_live_edge_graph(
    const Graph &G,
    uint64_t seed,
    double INFECT_PROB = 0.2
) {
    const int N = G.nodes.size();
    std::vector<std::vector<int>> live_adj(N);

    std::mt19937 rng(seed);

    for (int v = 0; v < N; ++v) {
        if (G.adj_list[v].empty())
            continue;

        // Uniformly choose ONE incoming neighbor u
        double r = std::uniform_real_distribution<double>(0.0, 1.0)(rng);
        if (r > INFECT_PROB)
            continue;

        std::uniform_int_distribution<int> dist(0, G.adj_list[v].size() - 1);
        auto it = G.adj_list[v].begin();
        std::advance(it, dist(rng));
        int u = *it;

        // Add directed live edge u -> v
        live_adj[u].push_back(v);
    }
    return live_adj;
}

std::vector<AdjList> sample_lt_live_edge_topologies(
    const Graph &G,
    int S,
    uint64_t seed)
{
    std::vector<AdjList> samples;
    samples.reserve(S);

    for (int s = 0; s < S; s++) {
        samples.push_back(
            sample_lt_live_edge_graph(G, seed + s)
        );
    }

    return samples;
}
