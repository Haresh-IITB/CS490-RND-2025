#ifndef DISTANCE_H
#define DISTANCE_H

#include<vector> 
#include<cmath>
#include<utility>

inline double distance(const std::pair<double,double> & p1, const std::pair<double,double> & p2) ; 

std::vector<std::vector<double>> pairwise_distance(const std::vector<std::pair<double,double>> & points) ; 

#endif