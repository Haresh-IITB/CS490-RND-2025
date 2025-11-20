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
    std::vector<bool> isVaccinable(num_nodes, true); // true -> available to pick (not yet vaccinated)
    std::vector<bool> vaccinated_nodes(num_nodes, false);

    // Build an infected mask to skip infected nodes during candidate selection
    std::vector<char> isInfectedMask(num_nodes, 0);
    for (int v : infected_nodes) if (v >= 0 && v < num_nodes) isInfectedMask[v] = 1;

    for (int i = 0; i < k; ++i) {
        int max_saved = -1;
        int max_saved_idx = -1;
        int defaultSavedNodes = evaluator(G, isVaccinable, -1, infected_nodes);

        for (int j = 0; j < num_nodes; ++j) {
            // Skip nodes already selected for vaccination or infected nodes
            if (!isVaccinable[j] || isInfectedMask[j]) continue;
            int extraSaved = evaluator(G, isVaccinable, j, infected_nodes) - defaultSavedNodes;
            if (extraSaved >= max_saved) {
                max_saved = extraSaved;
                max_saved_idx = j;
            }
        }

        if (max_saved_idx != -1) {
            vaccinated_nodes[max_saved_idx] = true;
            isVaccinable[max_saved_idx] = false;
            std::cout << "Greedy step " << (i+1) << "/" << k
                      << ": Vaccinated node " << max_saved_idx
                      << " (Saved " << max_saved << " more nodes)" << std::endl;
        } else {
            std::cout << "Greedy step " << (i+1) << "/" << k
                      << ": No more vaccinable nodes found." << std::endl;
            break;
        }
    }

    std::vector<int> result;
    for (int i = 0; i < num_nodes; ++i)
        if (vaccinated_nodes[i]) result.emplace_back(i);
    return result;
}
