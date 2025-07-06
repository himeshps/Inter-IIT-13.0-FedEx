#include "genetic.h"

using namespace std;

// Generates a set of k random indices from a range [0, n-1]
set<int> Genetic::RandomIndices(int n, int k)
{
    uniform_int_distribution<> dis(0, n - 1);
    set<int> random_indices;

    while (random_indices.size() < k)
    {
        random_indices.insert(dis(random_gen));
    }

    return random_indices;
}

// Calculates the fitness of a solution by evaluating the packing arrangement
int Genetic::FitnessFunction(Encoding solution)
{
    // Convert the encoding to dicrete orientations (0 to 5 - 6 possible orientations)
    for (int i = 0; i < num_packets; i++)
    {
        solution.orientation[i] = static_cast<int>(floor(solution.orientation[i] * 6));
    }

    // Generate the ordering of packages by sorting according to value in encoding
    vector<int> permutation(num_packets);

    iota(permutation.begin(), permutation.end(), 1);

    sort(permutation.begin(), permutation.end(), [&solution](size_t a, size_t b)
         { return solution.sequence[a - 1] < solution.sequence[b - 1]; });

    // Pack the packages and compute the total cost incurred
    Sorter t;
    t.val = [](Box a, Box b)
    { return true; };
    vector<Box> b;
    for (int i : permutation)
    {
        b.emplace_back(convertPacketToBox(packets[i - 1]));
    }
    vector<Uld> u;
    For(i, ulds.size()) u.emplace_back(convertULDToUld(ulds[i]));
    Merit Residue;
    Residue.val = residueFunc;
    Solver s(t, Residue, b, u);
    s.solve();
    return s.cost();
}

// Warm start function to initialize the population with random solutions
vector<Encoding> Genetic::Warmstart(int factor)
{
    cout << "Generation 1 started." << endl;

    uniform_real_distribution<> dist_priority(0.0, 0.5), dist_economy(0.5, 1.0);

    vector<Encoding> population(factor * population_size);
    for (int i = 0; i < population.size(); i++)
    {
        population[i].sequence.resize(num_packets), population[i].orientation.resize(num_packets);
        for (int j = 0; j < num_packets; j++)
        {
            if (packets[j].type == Priority)
            {
                population[i].sequence[j] = dist_priority(random_gen);
                population[i].orientation[j] = dist_priority(random_gen);
            }
            else
            {
                population[i].sequence[j] = dist_economy(random_gen);
                population[i].orientation[j] = dist_economy(random_gen);
            }
        }
        population[i].fitness = FitnessFunction(population[i]);
    }
    vector<int> indices(factor * population_size);

    iota(indices.begin(), indices.end(), 0);

    sort(indices.begin(), indices.end(), [&population](size_t a, size_t b)
         { return population[a].fitness > population[b].fitness; });

    // Select the top solutions to generate an initial population
    vector<Encoding> initial_population(population_size);
    for (int i = 0; i < population_size; i++)
    {
        initial_population[i] = population[indices[i]];
    }

    return initial_population;
}

// Selects parents for crossover using tournament selection
pair<Encoding, Encoding> Genetic::SelectParents(const vector<Encoding> &current_population, int tournament_factor)
{
    set indices = RandomIndices(population_size, tournament_factor);

    vector<pair<int, int>> fitness_index_pair;

    for (int index : indices)
    {
        fitness_index_pair.push_back({current_population[index].fitness, index});
    }

    sort(fitness_index_pair.begin(), fitness_index_pair.end(), greater<pair<int, int>>());

    // Return the two best solutions as parents
    return {current_population[fitness_index_pair[0].second], current_population[fitness_index_pair[1].second]};
}

// Performs crossover to create offspring from the current population
vector<Encoding> Genetic::Crossover(const vector<Encoding> &current_population, int num_offsprings)
{
    vector<Encoding> offsprings;
    for (int i = 1; i <= num_offsprings; i++)
    {
        pair<Encoding, Encoding> parents = SelectParents(current_population); // Select parents using tournament selection
        set<int> indices = RandomIndices(num_packets, 2);
        int start = *indices.begin(), end = *indices.rbegin();
        vector<double> seq(num_packets), orient(num_packets);
        // Perform the crossover by combining sequences and orientations of the parents
        for (int j = 0; j < num_packets; j++)
        {
            if ((j < start) || (j > end))
            {
                seq[j] = parents.second.sequence[j];
                orient[j] = parents.second.orientation[j];
            }
            else
            {
                seq[j] = parents.first.sequence[j];
                orient[j] = parents.first.orientation[j];
            }
        }
        // Calculate the fitness of the offspring and add it to the population
        int fitness = FitnessFunction(Encoding(seq, orient, INT_MIN));
        offsprings.push_back(Encoding(seq, orient, fitness));
    }
    return offsprings;
}

// Generates a neighboring solution by swapping a sequence of packets and modifying orientation slightly
Encoding Genetic::GenerateNeighbor(const Encoding &solution, int neighborhood_range)
{
    Encoding neighbor = solution;

    uniform_int_distribution<int> index_dist(0, num_packets - 1);
    uniform_real_distribution<double> change_dist(-0.1, 0.1);

    const int num_swaps = 1;
    set<int> swap_indices = RandomIndices(num_packets, num_swaps);

    // Perfrom swaps in some neighbourhood
    for (int i : swap_indices)
    {
        int lower_bound = max(0, i - neighborhood_range);
        int upper_bound = min(num_packets - 1, i + neighborhood_range);
        uniform_int_distribution<int> neighbor_dist(lower_bound, upper_bound);

        int j = neighbor_dist(random_gen);

        swap(neighbor.sequence[i], neighbor.sequence[j]);
    }

    // Slighly perturb the orientation at a random index
    int k = index_dist(random_gen);
    neighbor.orientation[k] = max(0.0, min(1.0, neighbor.orientation[k] + change_dist(random_gen)));

    neighbor.fitness = FitnessFunction(neighbor);

    return neighbor;
}

// Performs Tabu Search to improve the solution by exploring the neighborhood while avoiding previously visited solutions.
void Genetic::TabuSearch(Encoding &solution, int max_iter, int tabu_size, int neighborhood_size)
{
    deque<Encoding> tabu_list; // List of previously visited solutions to avoid

    Encoding best_solution = solution;

    // Iterate for a given number of iterations to improve the solution.
    for (int iter = 0; iter < max_iter; ++iter)
    {
        vector<Encoding> neighbors;

        for (int i = 0; i < neighborhood_size; ++i)
        {
            neighbors.push_back(GenerateNeighbor(solution)); // Generate neighbors by modifying the solution
        }

        // Select the best neighbor considering the tabu list
        Encoding best_neighbor = solution;
        int best_neighbor_fitness = INT_MIN;
        for (const auto &neighbor : neighbors)
        {
            bool is_tabu = false;
            for (const auto &tabu_solution : tabu_list)
            {
                if (neighbor.sequence == tabu_solution.sequence && neighbor.orientation == tabu_solution.orientation)
                {
                    is_tabu = true;
                    break;
                }
            }

            // If the neighbor is not tabu or improves upon the best solution (aspiration criterion), select it
            if (!is_tabu || neighbor.fitness > best_solution.fitness)
            {
                if (neighbor.fitness > best_neighbor_fitness)
                {
                    best_neighbor = neighbor;
                    best_neighbor_fitness = neighbor.fitness;
                }
            }
        }

        tabu_list.push_back(best_neighbor);

        // Remove the least recent solution from the list if size exceeded
        if (tabu_list.size() > tabu_size)
        {
            tabu_list.pop_front();
        }

        // If the best neighbor improves the best solution, update it
        if (best_neighbor_fitness > best_solution.fitness)
        {
            best_solution = best_neighbor;
        }
    }

    solution = best_solution;
}

// Applies mutation to the current population with a certain probability
void Genetic::Mutation(vector<Encoding> &current_population, double mutation_prob)
{
    uniform_real_distribution<> dist(0.0, 1.0);

    for (int i = 0; i < current_population.size(); i++)
    {
        if (dist(random_gen) < mutation_prob)
        {
            TabuSearch(current_population[i]);
        }
    }
}

// Constructor
Genetic::Genetic(const vector<ULD> &ulds, const vector<Packet> &packets, int n, int num_gen, int pop_size)
    : ulds(ulds), packets(packets), num_packets(n), num_generations(num_gen), population_size(pop_size)
{
    unsigned int random_seed = 42;
    random_gen = mt19937(random_seed);
}

// Driver code
void Genetic::Execute()
{
    cout << "Execution started. Total generations: " << num_generations << endl;

    ofstream top_solutions_file("top_solutions.txt");

    vector<Encoding> current_population = Warmstart();

    for (int i = 2; i <= num_generations; i++)
    {
        cout << "Generation " << i << " started." << endl;

        vector<Encoding> new_generation(population_size);
        int num_elites = population_size * ELITISM_PERCENTAGE;
        for (int j = 0; j < num_elites; j++)
        {
            new_generation[j] = current_population[j];
        }
        vector<Encoding> offsprings = Crossover(current_population, population_size - num_elites);
        for (int j = num_elites; j < population_size; j++)
        {
            new_generation[j] = offsprings[j - num_elites];
        }

        Mutation(new_generation);

        sort(new_generation.begin(), new_generation.end(), [](Encoding a, Encoding b)
             { return a.fitness > b.fitness; });

        top_solutions_file << "Generation " << i << ":\n";
        for (int j = 0; j < min(BEST_K_SOLNS, (int)new_generation.size()); j++)
        {
            Encoding &solution = new_generation[j];

            vector<int> transformed_orientation(num_packets);
            for (int k = 0; k < num_packets; k++)
            {
                transformed_orientation[k] = static_cast<int>(floor(solution.orientation[k] * 6));
            }

            vector<int> permutation(num_packets);
            iota(permutation.begin(), permutation.end(), 0);
            sort(permutation.begin(), permutation.end(), [&solution](size_t a, size_t b)
                 { return solution.sequence[a] < solution.sequence[b]; });

            top_solutions_file << "  Solution " << j + 1 << " Fitness: " << solution.fitness << "\n";
            top_solutions_file << "    Sequence: ";
            for (int k = 0; k < num_packets; k++)
            {
                top_solutions_file << permutation[k] << " ";
            }

            top_solutions_file << "\n";
            top_solutions_file << "    Orientation: ";
            for (int k = 0; k < num_packets; k++)
            {
                top_solutions_file << transformed_orientation[k] << " ";
            }
            top_solutions_file << "\n";
        }
        top_solutions_file << "\n";

        current_population = new_generation;

        cout << "Best Fitness of Generation " << i << ": " << new_generation[0].fitness << endl;
    }

    top_solutions_file.close();

    cout << "Execution completed." << endl;
}
