#ifndef CONFIG_H 
#define CONFIG_H

#include<vector>
#include<fstream>
#include<string>
#include <sstream>
#include <cctype> 

struct Config{
    std::vector<int> nodes ; 
    double alpha, beta ; 
    std::string model ; 
    std::vector<int> k_budget, infected ; 
};

struct Config initialiseConfig(std::string fileName) ; 
void trim_spaces(std::string & line) ; 
void parseList(std::string & line, std::vector<int> & v)  ;

#endif 