#ifndef MAIN_HELPERS_H
#define MAIN_HELPERS_H

#include <vector>
#include <set>
#include <string>
#include <functional>
#include <utility>
#include <cstdint>
#include <cmath>
#include "waxman-graph.h"

// -------------------- Schedule Types --------------------
enum class ScheduleType {
    UNIFORM,
    FRONT_LOADED,   // Heavy initial vaccination
    BACK_LOADED,    // Save resources for later
    STATIC_ONE_SHOT // Apply all at t=0
};

// -------------------- Result Struct --------------------
struct StrategyResult {
    int saved_count;
    double time_ms;
    std::string strategy_name;
};

// -------------------- Simulator Function Types --------------------
using SimulatorFn = std::function<int(
    Graph &,
    const std::vector<bool> &,
    const int &,
    const std::vector<bool> &,
    const uint64_t &,
    const int &,
    const double
)>;

using SimulatorFnDynamic = std::function<int(
    Graph &,
    const std::vector<std::pair<int,int>> &,
    const std::vector<bool> &,
    const uint64_t &,
    const int &,
    const double
)>;

// -------------------- Helper Function Declarations --------------------

// Initial infected sampling
std::vector<int> sample_initial_infected(
    const Graph &G,
    double frac,
    uint64_t seed
);

// Budget schedule generation
std::vector<int> generate_budget_schedule(
    int total_k,
    int steps,
    ScheduleType type
);

// Static baseline runs
int run_baseline(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<int> &initial_vaccinated,
    SimulatorFn Simulator,
    uint64_t seed,
    int stepSize,
    double prob_infect
);

int run_baseline_with_seeds(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<int> &initial_vaccinated,
    SimulatorFn Simulator,
    const std::vector<uint64_t> &seeds,
    int stepSize,
    double prob_infect
);

// Dynamic baseline runs
int run_baseline(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<std::pair<int,int>> &initial_vaccinated,
    SimulatorFnDynamic Simulator,
    uint64_t seed,
    int stepSize,
    double prob_infect
);

int run_baseline_with_seeds(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<std::pair<int,int>> &initial_vaccinated,
    SimulatorFnDynamic Simulator,
    const std::vector<uint64_t> &seeds,
    int stepSize,
    double prob_infect
);

// Waxman parameter scaling
void scale_waxman_params(
    int N,
    double base_alpha,
    double base_beta,
    double &out_alpha,
    double &out_beta
);

// Utility: extract vaccinated node IDs
std::vector<int> extractVaccineNode(
    const std::vector<std::pair<int,int>> &vaccines
);

#endif // MAIN_HELPERS_H
