#include "parse_data.h"

using namespace std;

class Genetic
{
private:
    // Member variables to store ULDs (Unit Load Devices), packets, number of packets, generations, population size, and random number generator
    vector<ULD> ulds;
    vector<Packet> packets;
    int num_packets, num_generations, population_size;
    mt19937 random_gen;

    // Helper function - Generates a set of random indices of size 'k' from a range of size 'n'
    set<int> RandomIndices(int n, int k);

    // Fitness function to evaluate the quality of a given solution
    int FitnessFunction(Encoding solution);

    // Warm start function to initialize the population with random solutions
    vector<Encoding> Warmstart(int factor = WARMSTART_FACTOR);

    // Crossover function to combine two parent solutions into offspring
    vector<Encoding> Crossover(const vector<Encoding> &current_population, int num_offsprings);

    // Selects two parent solutions using a tournament selection method
    pair<Encoding, Encoding> SelectParents(const vector<Encoding> &current_population, int tournament_factor = TOURNAMENT_FACTOR);
    
    // Applies mutation to the current population with a certain probability
    void Mutation(vector<Encoding> &current_population, double mutation_prob = MUTATION_PROB);

    // Generates a neighboring solution based on the given solution
    Encoding GenerateNeighbor(const Encoding &solution, int neighborhood_range = NEIGHBORHOOD_RANGE);

    // Applies Tabu search to refine the current solution
    void TabuSearch(Encoding &solution, int max_iter = MAX_TABU_ITERS, int tabu_size = TABU_SIZE, int neighborhood_size = NEIGHBORHOOD_SIZE);

public:
    // Constructor
    Genetic(const vector<ULD> &ulds, const vector<Packet> &packets, int n = NUM_PACKETS, int num_gen = NUM_GENERATIONS, int pop_size = POPULATION_SIZE);

    // Main method to run the genetic algorithm
    void Execute();
};
