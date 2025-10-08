#include "Random_number_generator.h"

Random_number_generator :: Random_number_generator(){
    uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count(); // seed 
    std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
    rng.seed(ss) ;         
}

double Random_number_generator :: get_unif(){
    std::uniform_real_distribution<double> unif(0,1) ; 
    return unif(rng) ; 
}

double Random_number_generator :: get_unif(const double & a, const double & b){
    std::uniform_real_distribution<double> unif(a,b) ; 
    return unif(rng) ; 
}

double Random_number_generator :: get_normal(const double & mean, const double & variance){
    std::normal_distribution<double> gauss(mean,sqrt(variance)) ; 
    return gauss(rng) ; 
}

double Random_number_generator :: get_normal(){
    std::normal_distribution<double> gauss(0,1) ; 
    return gauss(rng) ; 
}

int Random_number_generator :: get_bernoulli(){
    std::bernoulli_distribution b(0.5) ; 
    return b(rng) ; 
}

int Random_number_generator :: get_bernoulli(const double & p){
    std::bernoulli_distribution b(p) ; 
    return b(rng) ; 
}
