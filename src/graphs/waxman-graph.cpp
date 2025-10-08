#include "waxman-graph.h"


Graph :: Graph(int n) : V(n), 
                    num_cities(0) {} ;
Graph :: Graph(int n, int k) : 
                    V(n),
                    num_cities(k) {} ; 

void Graph :: normalize_distance(){
    double max_dist = 0 ; 
    for(const std::vector<double> & v : pw_dist)
        for(const double & x : v)
            max_dist = std::max(x,max_dist) ; 

    for(std::vector<double> & v : pw_dist)
        for(double & x : v)
            x = x/max_dist ; 
}


std::vector<double> Graph :: generate_cities(){

    if(num_cities == 0)
        num_cities = std::min(15,V/10) ; // Capping the cities with 15 
    
    // Sample the cities
    Random_number_generator rng_x, rng_y ; 
    for(int i = 0 ; i<num_cities ; i++)
        cities.emplace_back(std::make_pair(rng_x.get_unif(),rng_y.get_unif())) ; 
    
    nodes_pos = cities ; // Copy the cities position 

    // Generate the variance of the cities 
    std::vector<double> variance ; 
    Random_number_generator variance_gen ; 
        
    for(int i = 0 ; i<num_cities ; i++)
        variance.emplace_back(variance_gen.get_unif(0.1,0.4)) ;  
    
    return variance ; 

}



void Graph :: generate_nodes(){
        
    std::vector<double> variance = this->generate_cities() ; 
    Random_number_generator rng_x, rng_y ; 

    // Generate the remaining nodes based on the variance 
    double total_var = 0 ; 
    for(double v : variance)
        total_var += v ; 
        
    for(int i = 0 ; i<num_cities ; i++){
        int curr_node_cnt = round((V-num_cities)*(variance[i]/total_var)) ; 
        
        for(int j = 0 ; j<curr_node_cnt ; j++){    
            double px = rng_x.get_normal(cities[i].first, variance[i]) ;
            double py = rng_y.get_normal(cities[i].second, variance[i]) ;
            nodes_pos.emplace_back(std::make_pair(px,py)) ; 
        }
    }

    // Update the nodes count 
    V = nodes_pos.size() ; 

    // Generate the threshold for the nodes 
    nodesThreshold.resize(V) ; 
    Random_number_generator rng ; 
    for(int i = 0 ; i<V ; i++)
        nodesThreshold[i] = rng.get_unif() ; 
}




void Graph :: edge_toss(){
    // Change the graph again by tossing the coins 
    Random_number_generator rng ; 
    for(int i = 0 ; i<V ; i++)
        for(int j = i+1 ; j<V ; j++)
            adj_matrix[i][j] = rng.get_bernoulli(prob_edge[i][j]) ;

    for(int i = 0 ; i<V ; i++)
        for(int j = i-1 ; j>=0 ; j--)
            adj_matrix[i][j] = adj_matrix[j][i] ; 
}




void Graph :: generate_edges(const double & alpha, const double & beta) {
    pw_dist = pairwise_distance(nodes_pos) ;
    this->normalize_distance() ; 

    prob_edge.assign(V, std::vector<double>(V,0)) ; 
    for(int i = 0 ; i<V ; i++)
        for(int j = i+1 ; j < V ; j++)
            prob_edge[i][j] = alpha*std::exp(-pw_dist[i][j]/beta) ; 

    for(int i = 0 ; i<V ; i++)
        for(int j = i-1 ; j >= 0; j--)
            prob_edge[i][j] = prob_edge[j][i] ; 

    // Generate initial edge by fliping the coins 
    adj_matrix.assign(V,std::vector<int>(V,0)) ;
    this->edge_toss() ; 
}





void Graph :: update_graph(){
    // Regenerate the edges 
    this->edge_toss() ; 
}


void Graph :: graph_info() {
    std::cout << "Number of cities: " << num_cities << "\n";
    std::cout << "Total nodes (including cities): " << nodes_pos.size() << "\n";

    std::cout << "\nCities:\n";
    for(size_t i = 0; i < cities.size(); i++)
        std::cout << "  [" << i << "] (" << cities[i].first << ", " << cities[i].second << ")\n";

    std::cout << "\nNodes:\n";
    for(size_t i = 0; i < nodes_pos.size(); i++)
        std::cout << "  [" << i << "] (" << nodes_pos[i].first << ", " << nodes_pos[i].second << ")\n";

    std::cout << "\nEdges:\n";
    for(size_t i = 0; i < adj_matrix.size(); i++)
        for(size_t j = i+1; j < adj_matrix.size(); j++)
            if(adj_matrix[i][j] != 0)
                std::cout << "  " << i << " -- " << j << "\n";
}




// Helps to visualise the graph 
void Graph :: save_nodes_to_file(const std::string &node_file="nodes.csv",const std::string &edge_file="edges.csv") {

    std::ofstream out_nodes(node_file);
    if(!out_nodes.is_open()) { std::cerr << "Error opening " << node_file << "\n"; return; }

    out_nodes << "x,y,type\n";
    for(auto &c : cities) out_nodes << c.first << "," << c.second << ",city\n";
    for(size_t i = cities.size(); i < nodes_pos.size(); i++)
        out_nodes << nodes_pos[i].first << "," << nodes_pos[i].second << ",node\n";
    out_nodes.close();

    // Save edges
    std::ofstream out_edges(edge_file);
    if(!out_edges.is_open()) { std::cerr << "Error opening " << edge_file << "\n"; return; }

    out_edges << "source,target\n";
    for(size_t i = 0; i < adj_matrix.size(); i++)
        for(size_t j = i+1; j < adj_matrix.size(); j++)
            if(adj_matrix[i][j] != 0)
                out_edges << i << "," << j << "\n";

    out_edges.close();
    std::cout << "Saved nodes to " << node_file << " and edges to " << edge_file << "\n";
}





// int main() {
//     int num_nodes = 150;      // Total nodes
//     double alpha = 0.05;      // Waxman alpha
//     double beta  = 0.3;      // Waxman beta

//     // Create the graph
//     Graph* g = new Graph(num_nodes);

//     // Generate cities and nodes
//     g->generate_nodes();

//     // Generate edges
//     g->generate_edges(alpha, beta);

//     // Print graph info (nodes, cities, edges)
//     g->graph_info();

//     // Save nodes and edges to files
//     g->save_nodes_to_file("nodes.csv", "edges.csv");

//     // // Print the edge probability sum 
//     // for(int i = 0 ; i<g->V ; i++){
//     //     double sum = 0 ; 
//     //     for(int j = 0 ; j<g->V ; j++){
//     //         sum += g->prob_edge[i][j] ; 
//     //     }
//     //     std::cout << "For node " << i << " " << sum << "\n" ;
//     // }


//     std::cout << "Graph saved. Use graph.py to visualize.\n";

//     delete g;
//     return 0;
// }