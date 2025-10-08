#include "waxman-graph.h"
#include "Random_number_generator.h"
#include <vector> 
#include <functional> 
#include <set> 

std::vector<int> Local_search(Graph & G, 
    const int & K, 
    const std::vector<int> & InfectedNodes, 
    std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int> &)> evaluator,
    int max_iter = 15)
{
    // Randomly generate a set of vaccianted nodes 
    Random_number_generator rng ; 
    std::vector<int> v(G.V) ; 
    std::iota(v.begin(),v.end(),0) ; 
    std::shuffle(v.begin(),v.end(),rng.rng) ; 
    v.resize(K) ; 

    // Initialisation of vectors and sets 
    std::set<int> S ; // set of vacinnated nodes 
    std::vector<bool> isVaccinable(G.V,1) ; 
    
    for(int i : InfectedNodes)
        isVaccinable[i] = 0 ; 
    for(int elem : v){
        isVaccinable[elem] = 0 ; 
        S.insert(elem) ; 
    }

    // Local Search for another vaccinable node 
    bool convereged = false ; 
    int iterCnt = 0 ; 

    while (!convereged && iterCnt < max_iter){
        convereged = true ; // Assume convereged 
        int currSavedCnt = evaluator(G,isVaccinable,-1,InfectedNodes) ; 

        int bestSavedCnt = currSavedCnt ; // Bare minimum you want this 
        int bestSavedIdx = -1 ; 
        int ReplacedNode = -1 ; 

        for(int u : S){
            isVaccinable[u] = 1 ; // Un-Vaccinate u
            for(int v = 0 ; v<G.V ; v++){
                if(!isVaccinable[v] || G.adj_matrix[u][v] == 0) continue ; 
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
        std::cout << "Searched for node " << bestSavedIdx << std::endl ; 
        iterCnt ++ ; 
    }


    std::vector<int> result ; 
    for(int elem : S)
        result.push_back(elem) ; 

    return result ; 
}   