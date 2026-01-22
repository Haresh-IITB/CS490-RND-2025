#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdint>

struct Config {

    // -------- Graph parameters --------
    double alpha = 0.05;
    double beta  = 0.20;
    double cutoff_prob = 1e-3;
    double prob_infect = 0.1;
    // -------- Experiment parameters --------
    std::vector<int> node_sizes;   // e.g. 64,128,256,512
    double initial_infected_percent = 0.10;
    double vaccination_budget_percent = 0.10;

    int T = 10;           // number of topologies
    int stepSize = 1;     // dynamic step size 
    int timegap = 3; 
    int batches = 3;      // number of batches in dynamic strategy
    uint64_t seed = 42;

    // -------- Model --------
    std::string diffusion_model = "IC"; // "IC" or "LT"
};


static inline std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

static inline std::vector<int> parse_int_list(const std::string &s) {
    std::vector<int> result;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, ',')) {
        result.push_back(std::stoi(trim(token)));
    }
    return result;
}


static inline bool load_config(const std::string &filename, Config &cfg) {

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open config file: "
                  << filename << "\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {

        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        auto pos = line.find('=');
        if (pos == std::string::npos)
            continue;

        std::string key   = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        // -------- Graph --------
        if (key == "alpha")
            cfg.alpha = std::stod(value);
        else if (key == "beta")
            cfg.beta = std::stod(value);
        else if (key == "cutoff_prob")
            cfg.cutoff_prob = std::stod(value);

        // -------- Experiment --------
        else if (key == "node_sizes")
            cfg.node_sizes = parse_int_list(value);
        else if (key == "initial_infected_percent")
            cfg.initial_infected_percent = std::stod(value);
        else if (key == "vaccination_budget_percent")
            cfg.vaccination_budget_percent = std::stod(value);
        else if (key == "T")
            cfg.T = std::stoi(value);
        else if (key == "stepSize")
            cfg.stepSize = std::stoi(value);
        else if (key == "seed")
            cfg.seed = static_cast<uint64_t>(std::stoull(value));
        else if (key == "timegap")
            cfg.timegap = std::stoi(value);
        else if (key == "batches")
            cfg.batches = std::stoi(value);
        else if (key == "prob_infect")
            cfg.prob_infect = std::stod(value);
        // -------- Model --------
        else if (key == "diffusion_model")
            cfg.diffusion_model = value;
    }

    // -------- Sanity checks --------
    if (cfg.node_sizes.empty()) {
        std::cerr << "Error: node_sizes not specified in config\n";
        return false;
    }

    if (cfg.diffusion_model != "IC" &&
        cfg.diffusion_model != "LT") {
        std::cerr << "Error: diffusion_model must be IC or LT\n";
        return false;
    }

    return true;
}

#endif // CONFIG_H
