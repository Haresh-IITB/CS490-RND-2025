#include "waxman-graph.h"
#include <vector>
#include <set>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iostream>

// #define DEBUG

#ifdef DEBUG
#define DBG(x) do { std::cerr << x << std::endl; } while(0)
#else
#define DBG(x) do {} while(0)
#endif

std::vector<double> iter_pg_oe(
    const Graph &G,
    const std::vector<double> &pageVal,
    const double &alpha)
{
    const int N = G.nodes.size();
    std::vector<double> newPageVal(N, 0.0);

    double dangling_sum = 0.0;
    for (int u = 0; u < N; u++) {
        if (G.adj_list[u].empty())
            dangling_sum += pageVal[u];
    }

    double dangling_contrib = dangling_sum / N;

    for (int u = 0; u < N; u++) {
        if (G.adj_list[u].empty()) continue;

        double share = pageVal[u] / G.adj_list[u].size();
        for (int v : G.adj_list[u]) {
            newPageVal[v] += alpha * share;
        }
    }

    double teleport = (1.0 - alpha) / N;
    for (int i = 0; i < N; i++) {
        newPageVal[i] += teleport + alpha * dangling_contrib;
    }

    double sum = 0.0;
    for (double v : newPageVal) sum += v;
    for (double &v : newPageVal) v /= sum;

    return newPageVal;
}

bool converged(
    const std::vector<double> &a,
    const std::vector<double> &b,
    const double &tol)
{
    double diff = 0.0;
    for (size_t i = 0; i < a.size(); i++)
        diff += std::abs(a[i] - b[i]);

    diff /= a.size();
    DBG("[PR] convergence diff=" << diff);
    return diff < tol;
}

std::vector<int> PageRank(
    Graph &G,
    const int &K,
    const std::vector<int> &InfectedNodes,
    const double &alpha,
    const double &tolerance,
    int max_iter = 100,
    bool method = true)
{
    const int N = G.nodes.size();
    DBG("[PR] O(E) PageRank start | N=" << N << " K=" << K);

    std::vector<double> pageVal(N, 1.0 / N);

    for (int iter = 0; iter < max_iter; iter++) {
        DBG("[PR] Iteration " << iter);
        auto newPageVal = iter_pg_oe(G, pageVal, alpha);

        if (converged(newPageVal, pageVal, tolerance)) {
            DBG("[PR] Converged at iteration " << iter);
            pageVal = newPageVal;
            break;
        }
        pageVal = newPageVal;
    }

    std::vector<int> nodes(N);
    std::iota(nodes.begin(), nodes.end(), 0);

    std::sort(nodes.begin(), nodes.end(),
        [&](int a, int b) {
            return pageVal[a] > pageVal[b];
        });

    std::set<int> infectedSet(InfectedNodes.begin(), InfectedNodes.end());
    std::vector<int> vaccinated;

    DBG("[PR] Final PageRank values:");
    for (int u : nodes) {
        DBG("Node " << u << " : " << pageVal[u]);
    }

    if (method) {
        DBG("[PR] Method 1: neighbor guarding");

        for (int u : nodes) {
            if (infectedSet.count(u) == 0) continue;

            for (int v : G.adj_list[u]) {
                if ((int)vaccinated.size() >= K) break;
                if (infectedSet.count(v) == 0)
                    vaccinated.push_back(v);
            }

            if ((int)vaccinated.size() >= K) break;
        }
    }
    
    else {
        DBG("[PR] Method 0: pure PageRank");

        for (int u : nodes) {
            if ((int)vaccinated.size() >= K) break;
            if (infectedSet.count(u) == 0)
                vaccinated.push_back(u);
        }
    }

    DBG("[PR] Vaccinated count = " << vaccinated.size());
    return vaccinated;
}
