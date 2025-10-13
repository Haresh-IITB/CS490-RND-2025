#include "waxman-graph.h"
#include <vector> 
#include <set> 

std::vector<double> iter_pg(const Graph & G, 
    const std::vector<double> & pageVal,
    const double & alpha)
{    
    std::vector<double> newPageVal(G.V) ; 

    for(int i = 0 ; i<G.V ; i++){
        double inPageSum = 0.0 ; 
        int inDegCnt = 0 ; 
        for(int j = 0 ; j<G.V ; j++){
            if(G.adj_matrix[i][j] <= 0) continue ; 
            inPageSum += pageVal[j] ; 
            inDegCnt ++ ;
        }
        newPageVal[i] = (1-alpha) + (alpha)*(inPageSum/((double)inDegCnt)) ; 
    }
    
    return newPageVal ; 
}

bool converged(const std::vector<double> & pg1, 
    const std::vector<double> & pg2,
    const double & tolerance)
{
    int n = pg1.size() ; 
    double diff = 0 ; 
    for(int i = 0 ; i<n ; i++) 
        diff += std::abs(pg1[i]-pg2[i]) ; 
    diff /= (double)n ;
    
    return (diff < tolerance) ? 1 : 0 ; 
}

std::vector<int> PageRank(Graph & G, 
    const int & K, 
    const std::vector<int> & InfectedNodes, 
    const double & alpha, 
    const double & tolerance,
    int max_iter = 100)
{
    // Select the top-k Pages apart from the InfectedNodes
    std::vector<double> pageVal(G.V,1.0) ; 
    for(int iter = 0 ; iter < max_iter ; iter ++){
        std::vector<double> newPageVal = iter_pg(G, pageVal, alpha) ; 
        if(converged(newPageVal, pageVal, tolerance)){
            pageVal = newPageVal ; 
            break ; 
        }else{
            pageVal = newPageVal ; 
        }
    }

    std::vector<int> nodes(G.V) ; 
    std::iota(nodes.begin(), nodes.end(), 0) ; 
    std::sort(nodes.begin(), nodes.end(), [&](const int & a, const int & b){
        return pageVal[a] > pageVal[b] ; 
    }) ; 

    // Select the top-K 
    std::set<int> InfectedNodesSet ; 
    for(int i : InfectedNodes)
        InfectedNodesSet.insert(i) ; 

    std::vector<int> vaccinated ; 
    for(int i = 0 ; i<G.V && vaccinated.size() < K ; i++){
        if(InfectedNodesSet.count(nodes[i]) == 0)
            vaccinated.emplace_back(nodes[i]) ; 
    }

    return vaccinated ; 
}