#ifndef STRATEGY_H
#define STRATEGY_H

#include "waxman-graph.h"
#include <set> 
#include <functional> 

/*
Input : G(V,E), Budget k, Infected nodes I 
Return : Indices of the vaccinated nodes 
*/

enum class RoundingMethod {
    TKR,
    IRP
};

void Greedy_Vaccination(Graph &G, 
                        const int &k, 
                        const std::vector<int> &initial_infected,
                        std::vector<int> &out_vaccinated_nodes,
                        std::function<int(
                            Graph &,
                            const std::vector<bool> &,
                            const int &,
                            const std::vector<bool> &,
                            const uint64_t &,
                            const int &
                        )> Simulator,
                        int stepSize,
                        int T) ;

void Local_Search(
    Graph & G,
    const int & k, 
    const std::vector<int> & initial_infected,
    std::vector<int> & out_vaccinated_nodes,
    std::function<int(
        Graph &,
        const std::vector<bool> &,
        const int &,
        const std::vector<bool> &,
        const uint64_t &,
        const int &
    )> Simulator,
    int stepSize,
    int T,
    int max_no_improve_iters = 50,
    const std::vector<int> & intial_vaccinated_nodes = std::vector<int>()
) ;

void HIll_Climbing(
    Graph & G,
    const int & k, 
    const std::vector<int> & initial_infected,
    std::vector<int> & out_vaccinated_nodes,
    std::function<int(
        Graph &,
        const std::vector<bool> &,
        const int &,
        const std::vector<bool> &,
        const uint64_t &,
        const int &
    )> Simulator,
    int stepSize,
    int T,
    int max_no_improve_iters = 50,
    const std::vector<int> & intial_vaccinated_nodes = std::vector<int>()
);

std::vector<int> PageRank(
    Graph &G,
    const int &K,
    const std::vector<int> &InfectedNodes,
    const double &alpha,
    const double &tolerance,
    int max_iter = 100,
    bool method = true); 

std::vector<int> solve_lp_vaccination(
    const std::vector<Graph> &samples,
    const std::vector<int> &initial_infected,
    int k,
    RoundingMethod method
) ;
    
    
#endif