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

        
        std::set<int> temp  = S ; 
        std::vector<bool> Considerable(G.V,1) ; // This helps to mark that if a node is already swapped by v then it most not be again swapped by v'
        
        std::vector<int> ReplacedNodes ; 
        std::vector<int> ReplacingNodes ;

        for(int u : S){
            isVaccinable[u] = 1 ; // Un-Vaccinate u

            int bestSavedCnt = currSavedCnt ; // Bare minimum you want this 
            int bestSavedIdx = -1 ; 
            
            for(int v = 0 ; v<G.V ; v++){
                if(!isVaccinable[v] || G.adj_matrix[u][v] == 0 || !Considerable[v]) 
                    continue ; 
                int SavedCntV = evaluator(G,isVaccinable,v,InfectedNodes) ; 
                if(SavedCntV > bestSavedCnt){
                    bestSavedCnt = SavedCntV ; 
                    bestSavedIdx = v ; 
                }
            }

            isVaccinable[u] = 0 ; // Re-vaccinate this 
            
            if(bestSavedIdx != -1){
                convereged = false ; 
                temp.erase(u) ; 
                temp.insert(bestSavedIdx) ; 
                ReplacedNodes.push_back(u) ; 
                ReplacingNodes.push_back(bestSavedIdx) ; 
                Considerable[bestSavedIdx] = 0 ; 
            }
        }

        S = temp ; 
        // Fix the isVaccinable vector 
        for(int i : ReplacedNodes)
            isVaccinable[i] = 1 ; 
        for(int i : ReplacingNodes)
            isVaccinable[i] = 0 ; 

        iterCnt ++ ; 
    }


    std::vector<int> result ; 
    for(int elem : S)
        result.push_back(elem) ; 

    return result ; 
}   