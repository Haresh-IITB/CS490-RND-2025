/*
The file aims to generate a waxmam - graph of n nodes 
--> The graph generated is geometrically aware, we also make centers to make the graph
--> Simulation of city helps to simulate real-life cities 
*/

#ifndef WAXMAN_GRAPH_H
#define WAXMAN_GRAPH_H

#include<iostream> 
#include<vector> 
#include<algorithm>
#include<utility>
#include<fstream>
#include "Random_number_generator.h"
#include "distance.h"

class Graph{

public : 
    int V ; 
    int num_cities ; 
    std::vector<std::pair<double,double>> cities ; 
    std::vector<std::pair<double,double>> nodes_pos ;  
    std::vector<std::vector<int>> adj_matrix ; // Using adj-matrix because has to deal with 
                                               // Insertion, deletion and no - changes  
                                               
    std::vector<std::vector<double>> prob_edge ; 
    // Updating probabilities  
    // Use the prob_edge for updation, if the result same as prev -> No change 
    //                                 else -> Change 
    std::vector<double> nodesThreshold ; // Used in the linear threshold model 
    
private :
    std::vector<std::vector<double>> pw_dist ; // Pair-wise distance 
    

public : 
    Graph(int n) ; 
    Graph(int n, int k) ;
    
    /*
    This methods add edges to the graph based on the waxman-formulation
    --> Assumption : that the nodes are distributed in a [0,0] to [1,1] rectangle and have k centers 
    */

    void normalize_distance() ;

    std::vector<double> generate_cities() ; 
    void generate_nodes() ; 
    void generate_edges(const double & alpha, const double & beta) ; 

    // Helps to visualise the graph 
    void graph_info() ;
    void save_nodes_to_file(const std::string &node_file,const std::string &edge_file) ; 

private : 
    // Dynamically changes the graph after a each step 
    void edge_toss() ; 

public :
    void update_graph() ; 
    
};

#endif