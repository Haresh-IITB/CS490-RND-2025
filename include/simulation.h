#ifndef SIMULATION_H
#define SIMULATION_H

#include "waxman-graph.h"
#include "Random_number_generator.h"
#include<cmath>
#include<set> 
#include<queue> 

int IC_Simulation(Graph &G,
                  const std::vector<bool> &Vaccinated_Node,
                  const int &newVaccinatedNode,
                  const std::vector<bool> &Infected_Node,
                  const uint64_t &seed,
                  const int &stepSize,
                  const double prob_infect) ; 


int LinearThreshold_Simulator(
    Graph & G,
    const std::vector<bool> & Vaccinated_Node,
    const int & newVaccinatedNode, 
    const std::vector<bool> & Infected_Node,
    const uint64_t & seed,
    const int & stepSize
) ;
            
#endif