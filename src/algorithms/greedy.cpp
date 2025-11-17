#include "waxman-graph.h"
#include <set>
#include <functional>
#include <iostream>

std::vector<int> greedy_vaccination(Graph & G,
                                  const int & k,
                                  const std::vector<int> & infected_nodes,
                                  std::function<int(Graph&, std::vector<bool>&, const int&, const std::vector<int> &)> evaluator
){
    int num_nodes = G.nodes.size(); 
    std::vector<bool> isVaccinable(num_nodes, true);
    std::vector<bool> vaccinated_nodes(num_nodes, false);

    // Cannot vaccinate already-infected nodes
    for(int v : infected_nodes)
      isVaccinable[v] = false;

    for(int i = 0 ; i<k ; i++){
        int max_saved = -1; // Use -1 to handle negative gains
        int max_saved_idx = -1;

        // Calculate the baseline saved nodes with the currently selected set
        int defaultSavedNodes = evaluator(G, isVaccinable, -1, infected_nodes);
        
        // Find the node that gives the best *marginal gain*
        for(int j = 0 ; j < num_nodes; j++){ 
            // Skip the nodes already selected or infected
            if(!isVaccinable[j])
                continue;

            // See how many more nodes are saved by adding node 'j'
            int extraSaved = evaluator(G, isVaccinable, j, infected_nodes) - defaultSavedNodes;
            
            if(extraSaved >= max_saved){
                max_saved = extraSaved;
                max_saved_idx = j;
            }
        }

        // If we found a valid node, vaccinate it
        if (max_saved_idx != -1) {
            vaccinated_nodes[max_saved_idx] = true;
            isVaccinable[max_saved_idx] = false;
             std::cout << "Greedy step " << (i+1) << "/" << k 
                       << ": Vaccinated node " << max_saved_idx 
                       << " (Saved " << max_saved << " more nodes)" << std::endl;
        } else {
            // No more vaccinable nodes left
            std::cout << "Greedy step " << (i+1) << "/" << k 
                      << ": No more vaccinable nodes found." << std::endl;
            break;
        }
    }

    // Collect the final list of vaccinated node IDs
    std::vector<int> result;
    for(int i = 0 ; i < num_nodes; i++) // Corrected: G.V -> G.nodes.size()
        if(vaccinated_nodes[i])
          result.emplace_back(i);

    return result;
}
