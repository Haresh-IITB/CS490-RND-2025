#ifndef SIMULATION_H
#define SIMULATION_H

#include "waxman-graph.h"
#include "Random_number_generator.h"
#include<cmath>
#include<set> 
#include<queue> 

int ltSimulation (Graph & G, std::vector<bool> & isVaccinable, const int & newVaccinatedNode, const std::vector<int> & infected_nodes) ; 
int IC_Simulation (Graph & G, std::vector<bool> & isVaccinable, const int & newVaccinatedNode, const std::vector<int> & infected_nodes) ; 

#endif