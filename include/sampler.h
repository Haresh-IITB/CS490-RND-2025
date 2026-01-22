#ifndef SAMPLER_H
#define SAMPLER_H

#include "waxman-graph.h"
#include <vector>
#include <random>
#include <cmath>

std::vector<Graph> sample_lt_live_edge_topologies(
    const Graph &G,
    int S,
    uint64_t seed) ; 

std::vector<Graph> sample_ic_live_edge_topologies(
    const Graph &G,
    int S,
    uint64_t seed,
    double prob_infect) ; 

#endif 