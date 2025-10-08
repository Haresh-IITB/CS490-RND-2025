// Computes the pair wise-distance between N points 
#include "distance.h"

inline double distance(const std::pair<double,double> & p1, const std::pair<double,double> & p2){
    return sqrt(pow(p1.first - p2.first, 2) + pow(p1.second - p2.second, 2)) ; 
}

std::vector<std::vector<double>> pairwise_distance(const std::vector<std::pair<double,double>> & points){
    int n = points.size() ; 
    std::vector<std::vector<double>> pw_dist(n,std::vector<double>(n,0)) ; 

    for(int i = 0 ; i < n ; i++)
        for(int j = i+1 ; j<n ; j++)
            pw_dist[i][j] = distance(points[i], points[j]) ; 

    for(int i = 0 ; i < n ; i++)
        for(int j = i-1 ; j>=0 ; j--)
            pw_dist[i][j] = pw_dist[j][i] ;  
    
    return pw_dist ; 
}