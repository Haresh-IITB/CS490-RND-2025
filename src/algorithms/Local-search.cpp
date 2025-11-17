#include "Random_number_generator.h" 
#include "waxman-graph.h"
#include <vector> 
#include <functional> 
#include <set> 
#include <numeric>   
#include <algorithm> 
#include <iterator>  
#include <iostream>  

std::vector<int> Local_search(Graph & G, 
    const int & K, 
    const std::vector<int> & InfectedNodes, 
    std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int> &)> evaluator,
    int max_iter)
{
    int num_nodes = G.nodes.size(); 
    
    // Create a random initial set S of K nodes ---

    // Get a vector of all node IDs
    std::vector<int> v(num_nodes); 
    std::iota(v.begin(), v.end(), 0); 

    // Shuffle using the graph's main RNG
    std::shuffle(v.begin(), v.end(), G.rng_gen->rng); 

    std::set<int> S ; // set of vacinnated nodes 
    std::vector<bool> isVaccinable(num_nodes, true); 
    
    // Mark infected nodes as not vaccinable
    for(int i : InfectedNodes)
        isVaccinable[i] = false; 

    // Populate initial set S, ensuring no infected nodes are chosen
    int added = 0;
    for (int node_id : v) {
        if (added >= K) break;
        if (isVaccinable[node_id]) { // if not infected
            S.insert(node_id);
            isVaccinable[node_id] = false; // Now it's not "vaccinable" (it's already vaccinated)
            added++;
        }
    }

    std::cout << "  LS: Starting with random set of " << S.size() << " nodes." << std::endl;

    // ---  Run Local Search ---
    bool converged = false; 
    int iterCnt = 0; 

    while (!converged && iterCnt < max_iter){
        converged = true; // Assume convereged 
        
        // Get the score for the *start* of this iteration
        int currSavedCnt = evaluator(G, isVaccinable, -1, InfectedNodes); 
        std::cout << "  LS iter " << (iterCnt+1) << ": Starting score " << currSavedCnt << std::endl;

        std::set<int> temp_S = S; // A temporary set to hold swaps for this iteration
        std::vector<bool> Considerable(num_nodes, true); 

        std::vector<int> ReplacedNodes; 
        std::vector<int> ReplacingNodes;

        // Create a copy to iterate over, as S (via temp_S) will be modified
        std::vector<int> s_copy(S.begin(), S.end());

        for(int u : s_copy){ // For each node 'u' in the vaccinated set
            
            // Check if u was already swapped out in this iteration
            if (temp_S.find(u) == temp_S.end()) {
                continue;
            }

            isVaccinable[u] = true; // Temporarily un-vaccinate u

            int bestSavedCnt = -1; // Find the best swap *for this u*
            int bestSavedIdx = -1; 
            
            //  Local Search only swaps with *neighbors*
            for(int v : G.adj_list[u]){ 
                // if v is available, and hasn't been used in a swap this iter
                if(!isVaccinable[v] || !Considerable[v]) 
                    continue; 
                
                // Score for the set (S - {u}) + {v}
                int SavedCntV = evaluator(G, isVaccinable, v, InfectedNodes); 
                if(SavedCntV > bestSavedCnt){
                    bestSavedCnt = SavedCntV; 
                    bestSavedIdx = v; 
                }
            }

            isVaccinable[u] = false; // Re-vaccinate u
            
            // If we found a swap that's better than the iteration's start score
            if(bestSavedIdx != -1 && bestSavedCnt > currSavedCnt){
                std::cout << "    LS Swap found: " << u << " (out) for " 
                          << bestSavedIdx << " (in). New score: " << bestSavedCnt << std::endl;
                
                converged = false; // We made a change, so we are not converged
                temp_S.erase(u); 
                temp_S.insert(bestSavedIdx); 
                
                ReplacedNodes.push_back(u); 
                ReplacingNodes.push_back(bestSavedIdx); 
                Considerable[bestSavedIdx] = false; // Don't swap this new node *in this iteration*
                
                currSavedCnt = bestSavedCnt; 
            }
        }

        S = temp_S; // Commit all changes from this iteration
        // Fix the isVaccinable vector
        for(int i : ReplacedNodes)
            isVaccinable[i] = true; 
        for(int i : ReplacingNodes)
            isVaccinable[i] = false; 

        iterCnt++; 
    }

    if (converged && iterCnt > 0) {
        std::cout << "  Local search converged after " << iterCnt << " iterations." << std::endl;
    } else if (iterCnt == max_iter) {
        std::cout << "  Local search stopped at max_iter " << max_iter << "." << std::endl;
    }

    std::vector<int> result; 
    for(int elem : S)
        result.push_back(elem); 

    return result; 
}