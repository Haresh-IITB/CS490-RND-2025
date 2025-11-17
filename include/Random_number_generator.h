#ifndef RANDOM_NUMBER_GENERATOR_H
#define RANDOM_NUMBER_GENERATOR_H

#include <random>
#include <chrono>
#include <cmath>
#include <cstdint> // Include for uint64_t

// Random number generator
struct Random_number_generator {
    
    std::mt19937_64 rng;

    // Default constructor (seeds from time)
    Random_number_generator() ; 

    // Constructor that takes a specific seed
    Random_number_generator(uint64_t seed); 

    double get_unif() ; 
    double get_unif(const double & a, const double & b) ; 
    
    double get_normal() ; 
    double get_normal(const double & mean, const double & variance) ; 

    int get_bernoulli() ; 
    int get_bernoulli(const double & p) ; 
};

#endif