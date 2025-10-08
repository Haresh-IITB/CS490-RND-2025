#include "waxman-graph.h"
#include<vector> 
#include<queue> 

int IC_Simulation (Graph & G, std::vector<bool> & isVaccinable, const int & newVaccinatedNode, const std::vector<int> & infected_nodes){
    Random_number_generator rng ; 

    // Vacciante the new vaccinatedNode 
    if(newVaccinatedNode != -1)
        isVaccinable[newVaccinatedNode] = 0 ; 

    std::vector<bool> isInfectable(G.V,1) ;
    for(int v  = 0 ; v < isVaccinable.size() ; ++v)
        if (!isVaccinable[v]) 
            isInfectable[v] = 0 ;

    std::queue<int> q ; 
    int InfectedNodesCnt = infected_nodes.size() ; 

    // Initialise the queue
    for(int i : infected_nodes)
        q.push(i) ; 

    while(!q.empty()){
        int u = q.front() ; q.pop() ; 

        for(int v = 0 ; v<G.V ; ++v){
            if(G.adj_matrix[u][v] > 0 || !isInfectable[v])
                continue  ; 
            int isInfected = rng.get_bernoulli(G.prob_edge[u][v]) ;  
            if(isInfected){
                q.push(v) ; 
                isInfectable[v] = 0 ; 
                InfectedNodesCnt ++ ; 
            }
        }
    }

    // Reverse vaccination  
    if(newVaccinatedNode != -1)
        isVaccinable[newVaccinatedNode] = 1 ; 

    return G.V - InfectedNodesCnt ; 
}
    