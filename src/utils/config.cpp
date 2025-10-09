#include "config.h"
#include<iostream> 

void trim_spaces(std::string & line){
    int s = 0 , e = line.size() ;  
    while(s<line.size() && std::isspace(line[s]))
        s++ ;
    while(e>s && std::isspace(line[e-1]))
        e-- ; 
    line.erase(0,s) ; // Trim from start 
    line.erase(e-s) ; // Trim from end , end is shifted to e-s 
}

void parseList(std::string & line, std::vector<int> & v){
    line = line.substr(1,line.size()-2) ; 
    std::stringstream ss(line) ; 
    std::string temp ; 

    while(std::getline(ss, temp, ',')){
        trim_spaces(temp) ; 
        v.push_back(std::stoi(temp)) ; 
    }     
}

struct Config initialiseConfig(std::string fileName){
    Config parameters ; 

    std::ifstream fileConfig(fileName) ; 
    std::string line ; 

    while(std::getline(fileConfig, line)){
        trim_spaces(line) ; 
        if(line.empty() || line[0] == '#') continue ; 
        std::stringstream ss(line) ;
        std::string key , value ; 
        std::getline(ss, key, '=') ; 
        std::getline(ss, value) ; 

        if(key == "nodes")
            parseList(value, parameters.nodes) ; 
        else if(key == "alpha")
            parameters.alpha = std::stod(value) ; 
        else if(key == "beta")
            parameters.beta = std::stod(value) ; 
        else if(key == "k")
            parseList(value, parameters.k_budget) ; 
        else if(key == "model")
            parameters.model = value ; 
        else 
            parseList(value, parameters.infected) ;

    }

    return parameters ; 
}