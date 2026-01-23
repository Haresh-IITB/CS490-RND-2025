#ifndef DYNAMIC_STRATEGY_H
#define DYNAMIC_STRATEGY_H

#include "waxman-graph.h"
#include <iostream>
#include <iomanip>

enum class RoundingMethod {
    TKR,
    IRP
};

void Greedy_Vaccination_Dynamic(
    Graph &G, 
    int k, 
    const std::vector<int> &current_infected,
    const std::vector<bool> &pre_vaccinated_mask,
    std::vector<int> &out_new_vaccines,
    std::function<int(
                            Graph &,
                            const std::vector<bool> &,
                            const int &,
                            const std::vector<bool> &,
                            const uint64_t &,
                            const int &,
                            const double 
                        )> Simulator,
    int stepSize,
    int T,
    const double & Prob_infect
);

std::vector<std::pair<int,int>> run_rolling_horizon_strategy(
    Graph &base_graph,                  
    std::vector<int> initial_infected,
    const std::vector<int> &budget_schedule, 
    int time_step_gap,
    int num_samples_per_step,
    double prob_infect
) ;

#endif