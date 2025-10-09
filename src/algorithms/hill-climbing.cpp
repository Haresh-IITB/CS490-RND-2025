#include "waxman-graph.h"
#include "Random_number_generator.h"
#include <vector> 
#include <functional> 
#include <set> 

std::vector<int> hill_climbing(Graph & G, 
    const int & K, 
    const std::vector<int> & InfectedNodes, 
    std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int> &)> evaluator,
    std::vector<int> intitalVaccinatedNodes,
    int max_iter = 100)
{
    // Initialisation of vectors and sets 
    std::set<int> S ; // set of vacinnated nodes 
    for(int node : intitalVaccinatedNodes) S.insert(node) ; 
    std::vector<bool> isVaccinable(G.V,1) ; 
    
    for(int i : InfectedNodes)
        isVaccinable[i] = 0 ; 
    for(int elem : S){
        isVaccinable[elem] = 0 ; 
    }

    // Local Search for another vaccinable node 
    bool convereged = false ; 
    int iterCnt = 0 ; 

    // for(int i = 0 ; i<G.V ; i++){
    //     int sum = 0 ; 
    //     for(int j = 0 ; j<G.V ; j++)
    //         sum += G.adj_matrix[i][j] ; 
    //     std::cout << "Degree of " << i << " = " << sum << std::endl ;
    //     fflush(stdout) ;  
    // }

    while (!convereged && iterCnt < max_iter){
        convereged = true ; // Assume convereged 
        int currSavedCnt = evaluator(G,isVaccinable,-1,InfectedNodes) ; 

        int bestSavedCnt = currSavedCnt ; // Bare minimum you want this 
        int bestSavedIdx = -1 ; 
        int ReplacedNode = -1 ; 

        for(int u : S){
            isVaccinable[u] = 1 ; // Un-Vaccinate u
            for(int v = 0 ; v<G.V ; v++){
                if(!isVaccinable[v]) continue ; 
                int SavedCntV = evaluator(G,isVaccinable,v,InfectedNodes) ; 
                if(SavedCntV > bestSavedCnt){
                    bestSavedCnt = SavedCntV ; 
                    bestSavedIdx = v ; 
                    ReplacedNode = u ; 
                }
            }
            isVaccinable[u] = 0 ; // Re-vaccinate this 
        }

        if(bestSavedIdx != -1){
            convereged = false ; 
            S.erase(ReplacedNode) ; 
            S.insert(bestSavedIdx) ; 
            isVaccinable[ReplacedNode] = 1 ; 
            isVaccinable[bestSavedIdx] = 0 ;
        }
        // std::cout << "Searched for node " << bestSavedIdx << std::endl ; 
        iterCnt ++ ; 
    }


    std::vector<int> result ; 
    for(int elem : S)
        result.push_back(elem) ; 

    return result ; 
}   