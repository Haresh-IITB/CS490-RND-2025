#ifndef DYNAMIC_SIMULATION_H
#define DYNAMIC_SIMULATION_H

#include "waxman-graph.h"
#include <queue>
#include <iostream>
#include <iomanip>

const double P = 0.2 ; // Infection probability factor

enum InfectionModel {
    IC, // Independent Cascade
    LT  // Linear Threshold
} ;

void simulate_infection_spread_IC(
    const Graph &G, 
    std::vector<int> &infected_list, // Input/Output
    std::vector<bool> &infected_mask,
    const std::vector<bool> &vaccinated_mask, 
    int steps, 
    uint64_t seed,
    double prob_infect
) ;

int IC_Simulation_test(Graph &G,
                  const std::vector<std::pair<int,int>> &Vaccinated_Node,
                  const std::vector<bool> &Infected_Node,
                  const uint64_t &seed,
                  const int &stepSize,
                  const double prob_infect) ;
                
int IC_Simulation_search(Graph &G,
                  const std::vector<bool> &Vaccinated_Node,
                  const int &newVaccinatedNode,
                  const std::vector<bool> &Infected_Node,
                  const std::vector<int> & current_infected,
                  const uint64_t &seed,
                  const int &stepSize,
                  const double prob_infect) ;
                  
void simulate_infection_spread_LT(
    const Graph &G, 
    std::vector<int> &infected_list, 
    std::vector<bool> &infected_mask,
    const std::vector<bool> &vaccinated_mask, 
    int steps, 
    uint64_t seed,
    double prob_infect
) ;

int LT_Simulation_test(
    Graph &G,
    const std::vector<std::pair<int,int>> &Vaccinated_Node, // (node, time_id)
    const std::vector<bool> &Infected_Node,
    const uint64_t &seed,
    const int &stepSize
) ;

int LT_Simulation_search(
    Graph &G,
    const std::vector<bool> &Vaccinated_Node,
    const int &newVaccinatedNode,
    const std::vector<bool> &Infected_Node,
    const std::vector<int> &current_infected,
    const uint64_t &seed,
    const int &stepSize
) ;

#endif // DYNAMIC_SIMULATION_H