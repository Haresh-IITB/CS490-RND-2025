#include "waxman-graph.h"
#include "strategy.h"
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <iostream>

#include "gurobi_c++.h"

static GRBEnv GLOBAL_ENV;

struct LPModel {
    GRBModel model;
    int N, S;
    std::vector<GRBVar> I;
    std::vector<std::vector<GRBVar>> x;

    LPModel(int N_, int S_)
        : model(GLOBAL_ENV), N(N_), S(S_) {

        model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
        model.set(GRB_IntParam_OutputFlag, 0);
        model.set(GRB_IntParam_Presolve, 1);
        model.set(GRB_IntParam_Method, 2);    
        model.set(GRB_IntParam_Crossover, 0);

        I.resize(N);
        x.assign(N, std::vector<GRBVar>(S));
    }
};

static LPModel build_lp_model(
    const std::vector<Graph> &samples,
    const std::vector<int> &initial_infected,
    int k
) {
    const int S = samples.size();
    const int N = samples[0].nodes.size();

    LPModel lp(N, S);

    for (int i = 0; i < N; i++) {
        lp.I[i] = lp.model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS);
    }

    for (int i = 0; i < N; i++) {
        for (int s = 0; s < S; s++) {
            lp.x[i][s] = lp.model.addVar(
                0.0, 1.0, 1.0 / S, GRB_CONTINUOUS
            );
        }
    }

    lp.model.update();
    for (int s = 0; s < S; s++) {
        for (int i : initial_infected) {
            lp.model.addConstr(lp.x[i][s] == 1.0);
        }
    }

    for (int s = 0; s < S; s++) {
        const Graph &G = samples[s];
        for (int j = 0; j < N; j++) {
            for (int i : G.adj_list[j]) {
                lp.model.addConstr(
                    lp.x[i][s] - lp.x[j][s] + lp.I[i] >= 0.0
                );
            }
        }
    }

    GRBLinExpr budget = 0;
    for (int i = 0; i < N; i++) budget += lp.I[i];
    lp.model.addConstr(budget <= k);

    lp.model.update();
    return lp;
}

static std::vector<double> solve_lp(LPModel &lp) {
    lp.model.optimize();

    std::vector<double> scores(lp.N);
    for (int i = 0; i < lp.N; i++) {
        scores[i] = lp.I[i].get(GRB_DoubleAttr_X);
    }

    // for (int i = 0 ; i<lp.N ; i++) {
    //     std::cout << "Node " << i << " score: " << scores[i] << "\n";
    // }

    return scores;
}

static std::vector<int> run_tkr(
    const std::vector<Graph> &samples,
    const std::vector<int> &initial_infected,
    int k
) {
    LPModel lp = build_lp_model(samples, initial_infected, k);
    auto scores = solve_lp(lp);
    std::vector<int> nodes(scores.size());
    std::iota(nodes.begin(), nodes.end(), 0);

    std::sort(nodes.begin(), nodes.end(),
        [&](int a, int b) {
            return scores[a] > scores[b];
        });
    std::vector<int> vaccinated;
    for (int i = 0; i < (int)nodes.size() && (int)vaccinated.size() < k; i++) {
        vaccinated.push_back(nodes[i]);
    }

    return vaccinated;
}


static std::vector<int> run_irp(
    const std::vector<Graph> &samples,
    const std::vector<int> &initial_infected,
    int k
) {
    constexpr double EPS = 1e-4;

    const int N = samples[0].nodes.size();

    LPModel lp = build_lp_model(samples, initial_infected, k);

    std::unordered_set<int> fixed;
    for (int iter = 0; iter < k; iter++) {
        auto scores = solve_lp(lp);
        int best = -1;
        double best_val = 0.0;
        for (int i = 0; i < N; i++) {
            if (!fixed.count(i) && scores[i] > best_val) {
                best_val = scores[i];
                best = i;
            }
        }
        if (best < 0 || best_val < EPS) {
            break;
        }
        fixed.insert(best);
        lp.I[best].set(GRB_DoubleAttr_LB, 1.0);
        lp.I[best].set(GRB_DoubleAttr_UB, 1.0);
    }

    if ((int)fixed.size() < k) {

        auto scores = solve_lp(lp);

        std::vector<int> remaining;
        remaining.reserve(N);

        for (int i = 0; i < N; i++) {
            if (!fixed.count(i)) {
                remaining.push_back(i);
            }
        }

        std::sort(remaining.begin(), remaining.end(),
            [&](int a, int b) {
                return scores[a] > scores[b];
            });

        for (int i = 0;
             i < (int)remaining.size() && (int)fixed.size() < k;
             i++) {
            fixed.insert(remaining[i]);
        }
    }

    return std::vector<int>(fixed.begin(), fixed.end());
}


std::vector<int> solve_lp_vaccination(
    const std::vector<Graph> &samples,
    const std::vector<int> &initial_infected,
    int k,
    RoundingMethod method
) {
    return (method == RoundingMethod::TKR)
        ? run_tkr(samples, initial_infected, k)
        : run_irp(samples, initial_infected, k);
}