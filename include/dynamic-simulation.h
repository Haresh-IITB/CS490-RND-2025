#ifndef DYNAMIC_SIMULATION_H
#define DYNAMIC_SIMULATION_H

#include "waxman-graph.h"
#include <queue>
#include <iostream>
#include <iomanip>

const double P = 0.2 ; // Infection probability factor

int IC_Simulation(Graph &G,
                  const std::vector<bool> &Vaccinated_Node,
                  const int &newVaccinatedNode,
                  std::vector<bool> &Infected_Node,
                  std::vector<int> & Out_Newly_Infected,
                  const int & num_steps,
                  const uint64_t &seed,
                  const int &stepSize,
                  const double prob_infect) ;

#endif // DYNAMIC_SIMULATION_H