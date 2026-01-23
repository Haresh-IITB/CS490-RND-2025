#include "main-helper.h"

std::vector<int> sample_initial_infected(
    const Graph &G,
    double frac,
    uint64_t seed) {

    int N = G.nodes.size();
    int k = static_cast<int>(frac * N);

    std::set<int> s;
    Random_number_generator rng(seed);

    while ((int)s.size() < k) {
        s.insert(rng.get_int(0, N - 1));
    }
    return std::vector<int>(s.begin(), s.end());
}

std::vector<int> generate_budget_schedule(int total_k, int steps, ScheduleType type) {
    std::vector<int> schedule(steps, 0);
    if (type == ScheduleType::STATIC_ONE_SHOT) {
        schedule.resize(1) ; 
        schedule[0] = total_k; // All at once
        return schedule;
    }

    if (type == ScheduleType::UNIFORM) {
        int base = total_k / steps;
        int remainder = total_k % steps;
        for (int i = 0; i < steps; i++) {
            schedule[i] = base + (i < remainder ? 1 : 0);
        }
    } 
    else if (type == ScheduleType::FRONT_LOADED) {
        int current_k = total_k;
        for(int i=0; i<steps-1; ++i) {
            int alloc = std::max(1, (int)(current_k * 0.5)); // 50% of remaining
            schedule[i] = alloc;
            current_k -= alloc;
        }
        schedule[steps-1] = (current_k > 0) ? current_k : 0; // Dump remainder
    }
    else if (type == ScheduleType::BACK_LOADED) {
        std::vector<double> weights(steps);
        double sum_w = 0.0;

        for (int i = 0; i < steps; ++i) {
            weights[i] = (i + 1) * (i + 1);  // Quadratic growth
            sum_w += weights[i];
        }

        int allocated = 0;
        for (int i = 0; i < steps; ++i) {
            schedule[i] = static_cast<int>(
                std::round(total_k * weights[i] / sum_w)
            );
            allocated += schedule[i];
        }

        schedule[steps - 1] += (total_k - allocated);
    }


    return schedule;
}

static int run_baseline(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<int> &initial_vaccinated,
    SimulatorFn Simulator,
    uint64_t seed,
    int stepSize,
    double prob_infect) {
    int N = G.nodes.size();
    std::vector<bool> vaccinated(N, false);
    std::vector<bool> infected(N, false);

    for (int u : initial_infected)
        infected[u] = true;

    for (int u : initial_vaccinated)
        vaccinated[u] = true;

    int saved_cnt = Simulator(G, vaccinated, -1, infected, seed, stepSize, prob_infect); // saved nodes 
    return saved_cnt; // return number of saved nodes 
}

static int run_baseline_with_seeds(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<int> &initial_vaccinated,
    SimulatorFn Simulator,
    const std::vector<uint64_t> &seeds,
    int stepSize,
    double prob_infect) {

    int total = 0;
    for (uint64_t s : seeds) {
        total += run_baseline(
            G,
            initial_infected,
            initial_vaccinated,
            Simulator,
            s,
            stepSize,
            prob_infect
        );
    }

    int avg = total / seeds.size();
    return avg; // return average infected nodes
}

// Testing the vaccination under dynamic distribution 
static int run_baseline(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<std::pair<int,int>> &initial_vaccinated,
    SimulatorFnDynamic Simulator,
    uint64_t seed,
    int stepSize,
    double prob_infect) {
    int N = G.nodes.size();
    // std::vector<bool> vaccinated(N, false); 
    std::vector<bool> infected(N, false);

    for (int u : initial_infected)
        infected[u] = true;

    // for (int u : initial_vaccinated)
    //     vaccinated[u] = true;

    int saved_cnt = Simulator(G, initial_vaccinated, infected, seed, stepSize, prob_infect); // saved nodes 
    return saved_cnt; // return number of saved nodes 
}

static int run_baseline_with_seeds(
    Graph &G,
    const std::vector<int> &initial_infected,
    const std::vector<std::pair<int,int>> &initial_vaccinated,
    SimulatorFnDynamic Simulator,
    const std::vector<uint64_t> &seeds,
    int stepSize,
    double prob_infect) {

    int total = 0;
    for (uint64_t s : seeds) {
        total += run_baseline(
            G,
            initial_infected,
            initial_vaccinated,
            Simulator,
            s,
            stepSize,
            prob_infect
        );
    }

    int avg = total / seeds.size();
    return avg; // return average infected nodes
}


void scale_waxman_params(int N, double base_alpha, double base_beta, double &out_alpha, double &out_beta) {
    const double N_REF = 256.0; 
    out_alpha = base_alpha;
    double scale_factor = std::sqrt(N_REF) / std::sqrt((double)N);
    out_beta = base_beta * scale_factor;
}

std::vector<int> extractVaccineNode(const std::vector<std::pair<int,int>> & vaccines){
    std::vector<int> result ; 
    for(auto p : vaccines) result.push_back(p.first) ; 
    return result ; 
}
