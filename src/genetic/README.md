# Genetic Algorithm for 3D Bin Packing

This `Genetic` class implements a genetic algorithm to solve the 3D bin packing problem, where the goal is to pack packets into Unit Load Devices (ULDs) in an efficient manner. The algorithm uses evolutionary principles, including selection, crossover, mutation, and tabu search, to find optimal or near-optimal packing orders. The GA helps us determine good ordering and orientations of the packets which is used by the `Solver` class defined in `packing.h` to generate an efficient packing.

---

## Features

- **Warmstart Initialization**: Generates a diverse initial population to enhance exploration.
- **Fitness Evaluation**: Measures the quality of a packing solution based on the total cost based fitness function.
- **Selection and Crossover**: Employs tournament-based parent selection and segment-wise crossover to create new solutions.
- **Mutation with Tabu Search**: Incorporates tabu search during mutation to refine solutions and avoid local optima.
- **Customizable Parameters**: Allows tuning of population size, number of generations, mutation probability, and other key parameters.

---

## Key Structures

### `ULD` (Unit Load Device)
Represents the containers where packets are packed. Each ULD has the following properties:

- **Dimensions**: Length, width, and height.
- **Weight**: Weight of the ULD.
- **ID**: Unique identifier for the ULD.

### `Packet`
Represents the items to be packed into the ULDs. Each packet has the following properties:

- **Dimensions**: Length, width, and height.
- **Weight**: Weight of the packet.
- **Type**: Whether the packet is `Priority` or `Economy`.
- **Cost**: Optional cost associated with the packet.

### `Encoding`
Represents a solution encoding with two components:

- **Sequence**: The order of packets in the packing process.
- **Orientation**: The orientations of the packets.
- **Fitness**: The fitness score of the solution.

---

## Class Definition

```cpp
class Genetic
{
private:
    vector<ULD> ulds;                          // List of Unit Load Devices (ULDs)
    vector<Packet> packets;                    // List of packets to be packed
    int num_packets, num_generations, population_size;
    mt19937 random_gen;                        // Random number generator

    // Helper Functions
    set<int> RandomIndices(int n, int k);      // Generate a set of unique random indices
    int FitnessFunction(Encoding solution);    // Evaluate fitness of a given solution

    // Core GA Functions
    vector<Encoding> Warmstart(int factor = WARMSTART_FACTOR);                     // Generate initial population
    vector<Encoding> Crossover(const vector<Encoding> &current_population, int num_offsprings); // Perform crossover
    pair<Encoding, Encoding> SelectParents(const vector<Encoding> &current_population, int tournament_factor = TOURNAMENT_FACTOR); // Select parents for crossover
    
    void Mutation(vector<Encoding> &current_population, double mutation_prob = MUTATION_PROB); // Mutate population
    Encoding GenerateNeighbor(const Encoding &solution, int neighborhood_range = NEIGHBORHOOD_RANGE); // Generate a neighbor solution
    void TabuSearch(Encoding &solution, int max_iter = MAX_TABU_ITERS, int tabu_size = TABU_SIZE, int neighborhood_size = NEIGHBORHOOD_SIZE); // Refine solution using tabu search

public:
    Genetic(const vector<ULD> &ulds, const vector<Packet> &packets, int n = NUM_PACKETS, int num_gen = NUM_GENERATIONS, int pop_size = POPULATION_SIZE); // Constructor

    void Execute(); // Run the genetic algorithm
};
```

---

## File Structure

- **`main.cpp`**: Entry point for the program.
- **`genetic.cpp`**: Implements the Genetic Algorithm.
- **`packing.cpp`**: Packs the packages into the ULDs using an extreme point based heuristic.
- **`parse_data.cpp`**: Handles data parsing and input.

---

## Running Instructions

To run the Genetic Algorithm for 3D Bin Packing, you can compile and execute the program using the provided Makefile.

### Prerequisites

- Ensure that you have `g++` (GNU C++ Compiler) installed on your system.
- The program uses a Makefile to automate the build process.

### Steps

- ***Build:***
Run the following command to compile the code and generate the executable:

   ```bash
   make
   ```
   This will use the `g++` compiler to compile the source files (`main.cpp`, `genetic.cpp`, `packing.cpp`, `parse_data.cpp`) and create the executable named `solution`.

- ***Run the program:***
To execute the program, use the `run` target defined in the Makefile:

    ```bash
    make run
    ```

- ***Clean the build:***
If you want to clean up the object files and the executable, run:

    ```bash
    make clean
    ```
