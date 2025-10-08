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
){
    std::vector<bool> isVaccinable(G.V,1) ; 
    std::vector<bool> vaccinated_nodes(G.V,0) ;
    
    for(int v : infected_nodes) 
      isVaccinable[v] = 0 ; 
    
    for(int i = 0 ; i<k ; i++){
        int max_saved = 0 , max_saved_idx = 0 ; 
        int defualtSavedNodes = evaluator(G,isVaccinable,-1,infected_nodes) ; 
        for(int j = 0 ; j < G.V ; j++){
          // Skip the selected and infected nodes 
          if(!isVaccinable[j])
                continue ; 
          int extraSaved = evaluator(G,isVaccinable,j,infected_nodes) -  defualtSavedNodes ; 
          if(extraSaved >= max_saved){
            max_saved = extraSaved ; 
            max_saved_idx = j ; 
          }
        }
        vaccinated_nodes[max_saved_idx] = 1 ;
        isVaccinable[max_saved_idx] = 0 ;

        // std::cout << "Founded the node " << max_saved_idx << " \n" ; 
        
        // Update the graph after each step 
        // G.update_graph() ; // dynamic 
    }

    std::vector<int> result ; 
    for(int i = 0 ; i<G.V ; i++)
        if(vaccinated_nodes[i])
          result.emplace_back(i) ; 
    
    return result ; 
}