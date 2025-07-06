#include <bits/stdc++.h>

using namespace std;

#define NUM_ULDS 6
#define NUM_PACKETS 400
#define POPULATION_SIZE 50
#define NUM_GENERATIONS 10
#define WARMSTART_FACTOR 20
#define TOURNAMENT_FACTOR 5
#define ELITISM_PERCENTAGE 0.1
#define MUTATION_PROB 0.2
#define TABU_SIZE 5
#define MAX_TABU_ITERS 5
#define NEIGHBORHOOD_SIZE 10
#define NEIGHBORHOOD_RANGE 10
#define SPREAD_COST 5000
#define BEST_K_SOLNS 3

struct Dimensions
{
    int length, width, height;
};

struct Position
{
    int x, y, z;
};

enum PacketType
{
    Priority,
    Economy
};

struct ULD
{
    Dimensions dimensions;
    int id, weight;

    ULD(Dimensions dim, int i, int wt)
        : dimensions(dim), id(i), weight(wt) {}
};

struct Packet
{
    Dimensions dimensions;
    int id, weight, uld_num;
    PacketType type;
    optional<int> cost;

    Packet(Dimensions dim, int i, int wt, int num, PacketType t, optional<int> val = nullopt)
        : dimensions(dim), id(i), weight(wt), uld_num(num), type(t), cost(val) {}
};

struct Encoding
{
    vector<double> sequence, orientation;
    int fitness;

    Encoding() : sequence(), orientation(), fitness(0) {}

    Encoding(const vector<double> &seq, const vector<double> &orient, int fit)
        : sequence(seq), orientation(orient), fitness(fit) {}
};
