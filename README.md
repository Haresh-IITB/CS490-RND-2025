# Limiting Disease Spreading in Human Networks
This project is a C++ implementation of the models and algorithms described in the research paper "Limiting Disease Spreading in Human Networks" by Prof.Sujoy Bhore, Prof. Suraj Shetiya and Gargi Bakshi.

The simulation aims to identify an optimal set of k nodes to vaccinate in a network to minimize the spread of a disease, given an initial set of infected nodes.

@Author Code : Haresh Gupta 

## About The Project
This project explores vaccination strategies on spatially structured networks. It contrasts with traditional influence maximization problems by focusing on disease-spreading dynamics where the optimization function is neither submodular nor supermodular.

This implementation includes:

* Graph Generation: Spatially-aware Random Geometric graphs (ex : Waxman graphs)

* Disease Spread Models: The Independent Cascade (IC) and Linear Threshold (LT) models.

* Vaccination Algorithms: A Greedy approach and a Local Search heuristic to find the optimal set of nodes for vaccination.

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
* To run the simulation: **./bin/simulation config.txt** 
* This will generate a random graph that is spatially aware, and then perform the various vacciantion stratergies and store the results in the results/ directory.
* To visualize the graph generated, along with seeing which nodes are vaccinated and infected run **python3 scripts/visualise.py**
