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
    std::vector<bool> &Infected_Node,
    const std::vector<bool> &pre_vaccinated_mask,
    std::vector<int> &out_new_vaccines,
    std::function<int(
                            Graph &,
                            const std::vector<bool> &,
                            const int &,
                            const std::vector<bool> &,
                            const std::vector<int> &,
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

void Local_Search(
    Graph & G,
    const int & k, 
    const std::vector<int> & initial_infected,
    std::vector<bool> & infected_mask,
    const std::vector<bool> & pre_vaccinated_mask,
    std::vector<int> & out_vaccinated_nodes,
    std::function<int(
        Graph &,
        const std::vector<bool> &,
        const int &,
        const std::vector<bool> &,
        const std::vector<int> &,
        const uint64_t &,
        const int &,
        const double
    )> Simulator,
    int stepSize,
    int T,
    int max_no_improve_iters,
    const double & Prob_infect,
    const std::vector<int> & intial_vaccinated_nodes = std::vector<int>()
);

void Hill_Climbing(
    Graph & G,
    const int & k, 
    const std::vector<int> & initial_infected,
    std::vector<bool> & infected_mask,
    const std::vector<bool> & pre_vaccinated_mask,
    std::vector<int> & out_vaccinated_nodes,
    std::function<int(
        Graph &,
        const std::vector<bool> &,
        const int &,
        const std::vector<bool> &,
        const std::vector<int> &,
        const uint64_t &,
        const int &,
        const double
    )> Simulator,
    int stepSize,
    int T,
    int max_no_improve_iters,
    const double & Prob_infect,
    const std::vector<int> & intial_vaccinated_nodes = std::vector<int>()
) ;

std::vector<std::pair<int,int>> run_rolling_horizon_strategy_irp(
    Graph &base_graph,                  
    const std::vector<int> &initial_infected,
    const std::vector<int> &budget_schedule, 
    int time_step_gap,
    int num_samples_per_step,
    double prob_infect
) ;

#endif