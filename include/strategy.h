#ifndef STRATEGY_H
#define STRATEGY_H

#include "waxman-graph.h"
#include <set> 
#include <functional> 

/*
Input : G(V,E), Budget k, Infected nodes I 
Return : Indices of the vaccinated nodes 
*/

std::vector<int> greedy_vaccination(Graph & G, 
                      const int & k, 
                      const std::vector<int> & infected_nodes,
                      std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int> &)> evaluator
);

std::vector<int> Local_search(Graph & G, 
    const int & K, 
    const std::vector<int> & InfectedNodes, 
    std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int> &)> evaluator,
    int max_iter = 100000) ;

std::vector<int> hill_climbing(Graph & G, 
    const int & K, 
    const std::vector<int> & InfectedNodes, 
    std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int> &)> evaluator,
    std::vector<int> intitalVaccinatedNodes,
    int max_iter = 100);

std::vector<int> PageRank(Graph & G, 
    const int & K, 
    const std::vector<int> & InfectedNodes, 
    const double & alpha, 
    const double & tolerance,
    int max_iter = 100) ; 

#endif