# Limiting Disease Spreading in Human Networks
This project is a C++ implementation of the models and algorithms described in the research paper "Limiting Disease Spreading in Human Networks" by Prof.Sujoy Bhore, Prof. Suraj Shetiya and Gargi Bakshi.

The simulation aims to identify an optimal set of k nodes to vaccinate in a network to minimize the spread of a disease, given an initial set of infected nodes under batch distribution and dynamic settings.

@Author Code : Haresh Gupta 

## About The Project
This project explores vaccination strategies on spatially structured networks. It contrasts with traditional influence maximization problems by focusing on disease-spreading dynamics where the optimization function is neither submodular nor supermodular.

This implementation includes:

* Graph Generation: Spatially-aware Random Geometric graphs (ex : Waxman graphs)

* Disease Spread Models: The Independent Cascade (IC) and Linear Threshold (LT) models.

* Vaccination Algorithms: This project explore an alternate to current alogirithms like greedy or heurestics based (local search, hill climbing), it explores linear programming as an alternate which provides good tradeoff of run time and %nodes saved. 

* Model Simulation: This projects explore dynamic setting in contrast to the traditional static setting, by dynamic setting we mean that nodes migrate in the graph and edges modify accordingly, also the vaccination process is not done all at once but the vaccines are distributed in batches simulating real world. 

## Citation

This work is based on the following paper:

Gargi Bakshi, Sujoy Bhore, and Suraj Shetiya. Limiting Disease Spreading in Human Networks. 
<!-- arXiv:2503.22191v1 [cs.SI]. -->

## Getting Started
Follow these instructions to compile and run the simulation on your local machine.

### Prerequisites

* Standard used in this project is c++11

* make for automating the build process

* Python 3 for running helper scripts (e.g., visualization)

### Compilation

* Clone the repository : git clone https://github.com/Haresh-IITB/CS490-RND-2025/

* From the **project's root directory**, run the make command. This will compile all source files and place the executable in the bin/ directory.

## Usage
* Run the **./run_all_dynamic.sh**, to run all the algorithms with various diffusion model. The results are stores in the /results directory you may use the **./scripts/plot-csv.py** to generate plots for better visualisation 
* You could modify, the configuration files in the config/ for the factor like number of topologies, percent infected nodes and percent vaccinated nodes and observe the effect of the vaccination algorithms under various setting