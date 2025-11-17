#include "waxman-graph.h"
#include <set>
#include <functional>
#include <vector>
#include <iostream> 

std::vector<int> hill_climbing(Graph & G, 
    const std::vector<int> & InfectedNodes, 
    std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int> &)> evaluator,
    std::vector<int> initialVaccinatedNodes, 
    int max_iter)
{
    int num_nodes = G.nodes.size();
    
    std::set<int> S ; // set of vacinnated nodes 
    for(int node : initialVaccinatedNodes) S.insert(node) ; 
    
    // isVaccinable means "is available to be chosen as a replacement"
    std::vector<bool> isVaccinable(num_nodes, true); 
    
    for(int i : InfectedNodes)
        isVaccinable[i] = false; 
    for(int elem : S){
        isVaccinable[elem] = false; 
    }

    // Local Search for another vaccinable node 
    bool converged = false; 
    int iterCnt = 0 ; 

    while (!converged && iterCnt < max_iter){
        converged = true ; // Assume convereged 
        
        // Get baseline score for the current set S
        // We pass `isVaccinable` which already has S *removed* from the pool.
        int currSavedCnt = evaluator(G, isVaccinable, -1, InfectedNodes); 

        int bestSavedCnt = currSavedCnt; 
        int bestNodeToAdd = -1;    // The node 'v' to add
        int bestNodeToDrop = -1; // The node 'u' to remove

        for(int u : S){ // For each node 'u' in our vaccinated set
            isVaccinable[u] = true; // Temporarily "un-vaccinate" u, making it available
            
            // Now check every *other* node 'v' as a replacement
            for(int v = 0 ; v < num_nodes; v++){ 
                if(!isVaccinable[v]) continue; // Skip if 'v' is infected or already in S
                
                // Score for the set (S - {u}) + {v}
                // We test adding 'v' to the pool where 'u' is also available
                int SavedCntV = evaluator(G, isVaccinable, v, InfectedNodes); 
                
                if(SavedCntV > bestSavedCnt){
                    bestSavedCnt = SavedCntV; 
                    bestNodeToAdd = v; 
                    bestNodeToDrop = u; 
                }
            }
            isVaccinable[u] = false; // "Re-vaccinate" u before checking the next swap
        }

        if(bestNodeToAdd != -1){ 
            converged = false; 
            
            // Perform the swap
            S.erase(bestNodeToDrop); 
            S.insert(bestNodeToAdd); 
            
            // Update the "vaccinable" map for the next iteration
            isVaccinable[bestNodeToDrop] = true; 
            isVaccinable[bestNodeToAdd] = false;
            
            std::cout << "  HC iter " << (iterCnt+1) << ": Swapped " 
                      << bestNodeToDrop << " (out) for " 
                      << bestNodeToAdd << " (in). New score: " 
                      << bestSavedCnt << std::endl;
        }
        iterCnt++; 
    }

    if (converged && iterCnt > 0) {
         std::cout << "  Hill climbing converged after " << iterCnt << " iterations." << std::endl;
    } else if (iterCnt == max_iter) {
         std::cout << "  Hill climbing stopped at max_iter " << max_iter << "." << std::endl;
    } else {
         std::cout << "  Hill climbing found no improvement from initial set." << std::endl;
    }


    std::vector<int> result; 
    for(int elem : S)
        result.push_back(elem); 

    return result; 
}



