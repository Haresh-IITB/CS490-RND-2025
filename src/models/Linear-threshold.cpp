#include "waxman-graph.h"
#include "Random_number_generator.h"
#include<cmath>
#include<set> 
#include<queue> 

#define eps 1e-6

int ltSimulation (Graph & G, std::vector<bool> & isVaccinable, const int & newVaccinatedNode, const std::vector<int> & infected_nodes){
    
    if(newVaccinatedNode != -1)
        isVaccinable[newVaccinatedNode] = 0 ; 

    // Run the BFS from the infected nodes and calculate the threshold values for it's neighbour
    std::queue<int> q ; 
    std::vector<double> currInfectScore(G.V,0) ; 
    std::vector<bool> isInfectable(G.V, 1) ;  // Bool stores whether a node is infecteable or not
    int infectedNodesCnt = infected_nodes.size() ; 

    // Initialise the bfs_queue
    for(int i : infected_nodes)
        q.push(i) ; 

    // All the nodes who are not vacciable are either infected or vaccinated, so can't be infected further 
    for(int i = 0 ; i<G.V ; i++)
        if(!isVaccinable[i])
            isInfectable[i] = 0 ; 

    while(!q.empty()){
        int curr = q.front() ; q.pop() ; 

        for(int i = 0 ; i<G.V ; i++){
            // Cannot infect the node itself, if not neighbour, another infected node, the vaccinated nodes and the new infected nodes 
            if (i == curr || G.adj_matrix[curr][i] == 0 || !isInfectable[i]) 
                continue ; 

            currInfectScore[i] += G.prob_edge[curr][i] ; // Assuming wij prop to prob_edge
            
            // Add to the queue if it's a infected node 
            if(currInfectScore[i] >= G.nodesThreshold[i]){
                q.push(i) ; 
                infectedNodesCnt ++ ; 
                isInfectable[i] = false;
            }
        }
    }

    // Undo the insertion 
    if(newVaccinatedNode != -1)
        isVaccinable[newVaccinatedNode] = 1 ; 

    return G.V - infectedNodesCnt ; // Number of saved nodes  
}