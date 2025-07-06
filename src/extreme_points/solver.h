#include "bits/stdc++.h"
#define int long long
#define For(i, n) for (int i = 0; i < n; i++)
#define FOR(k, i, n) for (int i = k; i < n; i++)
#define vi vector<int>
#define maxP(a, b) (a.first > b.first ? a : b)
#define INF 10000000000000000
#define pii pair<int, int>
#define PRIORITY_MISS_COST 1000000
#define NEW_ULD_PRIORITY_COST 5000
#define RESIDUE_THRESHOLD 0
#define convertCoords(pt) pair<int, pair<int, pii>>(pt.box, pair<int, pii>(pt.x, pii(pt.y, pt.z)))
using namespace std;

#define SWAP_PAIRS 50

/*Define this to use EP2 based solving*/
#define EP2

bool check(const pair<pair<int, int>, pair<int, int>> &a, const pair<pair<int, int>, pair<int, int>> &b);
class Solver;
struct Box
{
    int l, b, h, weight, cost, ID;
    bool isPriority;
};
struct Sorter
{
    function<void(vector<Box> &data)> val;
};
struct coords
{
    int x, y, z, box;
};
struct Merit
{
    //    function<void(Solver*)>init;
    function<int(coords, Box, Solver *)> val;
};
struct Uld
{
    Box dim;
    int weight, maxWt, ID;
    coords com, maxBound;
};
class Solver
{
public:
    vi meritVar, sortVar;
    map<pair<int, pair<int, pii>>, pair<int, pii>> ep; //(box,(x,(y,z))):(x,(y,z))
    Merit merit;
    Sorter sorter;
    vector<pair<coords, Box>> placement; //(bottom left corner, dimensions)
    vector<set<int>> ULDPackages;        // sees which package is placed in which ULD
    vector<bool> ULDHasPriority;
    vector<Box> data;
    coords def;
    vector<Uld> ULDl;
    vector<set<pair<int, pair<pair<int, int>, pair<int, int>>>>> surfaces;

    int solveFrom = 0;
    int solveTill;

    // Data structures for additional EP creation
    function<bool(const int &, const int &)> compare_x;
    function<bool(const int &, const int &)> compare_y;
    function<bool(const int &, const int &)> compare_z;

    vector<set<int, function<bool(const int &, const int &)>>> ULD_sorted_x;
    vector<set<int, function<bool(const int &, const int &)>>> ULD_sorted_y;
    vector<set<int, function<bool(const int &, const int &)>>> ULD_sorted_z;

    function<bool(const int &, const int &)> compare_x_base;
    function<bool(const int &, const int &)> compare_y_base;
    function<bool(const int &, const int &)> compare_z_base;

    set<int, function<bool(const int &, const int &)>> ULD_blocking_boxes_x;
    set<int, function<bool(const int &, const int &)>> ULD_blocking_boxes_y;
    set<int, function<bool(const int &, const int &)>> ULD_blocking_boxes_z;

    // Constructor
    Solver(Sorter sorter_, Merit merit_, vector<Box> boxes, vector<Uld> ULD_);

    bool writeToFile(string filename);
    int cost();
    bool checkCollision(coords e, Box b);
    bool check_collision_2D(int x1_min, int x1_max, int y1_min, int y1_max, int x2_min, int x2_max, int y2_min, int y2_max);
    bool checkGravity(coords e, Box b);
    void gravity_pull(int i);
    void setSolveFrom(int _solveFrom);
    void setSolveTill(int _solveTill);
    void resetSolveTill();
    virtual void solve();
    void update(int i);

    vector<coords> getCOM();
    pair<int, pii> getResidueSpace(coords src);
    void initialise(coords &X, coords &ob, int x, int y, int z);

    coords beamprojectZNeg(coords ob1);
    coords beamprojectYNeg(coords ob1);
    coords beamprojectXNeg(coords ob1);
    coords beamprojectZPos(coords ob1);
    coords beamprojectYPos(coords ob1);
    coords beamprojectXPos(coords ob1);

    void addEP2(int i);
    void addEP(int i);
    void updateMaxBound(int i);
    void updateResidue(int i);

    int XBeamIntersectionWithBox(coords start, int ind);
    int YBeamIntersectionWithBox(coords start, int ind);
    int ZBeamIntersectionWithBox(coords start, int ind);
    int XRayIntersectionWithBox(coords start, int ind);
    int YRayIntersectionWithBox(coords start, int ind);
    int ZRayIntersectionWithBox(coords start, int ind);

    void projection_neg_x_advanced(vector<coords> &advanced_eps, coords start, int self_pid);
    void projection_neg_y_advanced(vector<coords> &advanced_eps, coords start, int self_pid);
    void projection_neg_z_advanced(vector<coords> &advanced_eps, coords start, int self_pid);
    coords rayProjectXNeg(coords start);
    coords rayProjectYNeg(coords start);
    coords rayProjectZNeg(coords start);
    coords rayProjectXPos(coords start);
    coords rayProjectYPos(coords start);
    coords rayProjectZPos(coords start);
};

/*
 *   Assumes that the IDs are incrementally available, use a map instead of vectors ahead if
 *   the IDs are not incrementally available.
 *   The implementation of the paper assumes that each box is packed into a bin and tries to find the minimum
 *   number of bins required. We often miss out on the packing of the box, how to handle that case is
 *   to be thought of ourselves.
 *   Also, all of these considerations are only for economy packages and not for priority packages.
 */
class ScoredSolver : public Solver
{
public:
    int iterations = 10, bestCost = numeric_limits<int>::min();
    double alpha = 0.1, beta = 0.1;
    vector<int> insertionOrder;
    // map<int, Box*> boxMap;
    vector<Uld> originalUldList;
    vector<Box *> boxMap;
    vector<int> bestSolution;

    /*Related to Scoring based Procedure*/
    vector<double> score;         /*Store the score correpsonding to each object*/
    vector<int> insertionCounter; /*Store how many times each ID has been introduced in a solution, and how many times it hasn't*/
    vector<int> lastInsertion;    /*Store the order of insertion in the last iteration. We store only economy packages for convenience*/
    set<int> lastInsertionSet;    /*Store the last insertion as a set for quick access*/

    set<int> economyPackages;     /*Store the economy packages IDs*/

    /*Caching related: We use caching to ensure that all the var*/
    map<pair<int, pair<int, pii>>, pair<int, pii>> cachedEp;
    vector<pair<coords, Box>> cachedPlacement;
    vector<bool> cachedULDHasPriority;
    vector<set<int>> cachedULDPackages;
    vector<Uld> cachedULDl;
    vector<set<pair<int, pair<pair<int, int>, pair<int, int>>>>> cachedSurfaces;

    vector<set<int, function<bool(const int &, const int &)>>> cachedULD_sorted_x;
    vector<set<int, function<bool(const int &, const int &)>>> cachedULD_sorted_y;
    vector<set<int, function<bool(const int &, const int &)>>> cachedULD_sorted_z;

    set<int, function<bool(const int &, const int &)>> cachedULD_blocking_boxes_x;
    set<int, function<bool(const int &, const int &)>> cachedULD_blocking_boxes_y;
    set<int, function<bool(const int &, const int &)>> cachedULD_blocking_boxes_z;

    ScoredSolver(Sorter sorter_, Merit merit_, vector<Box> boxes, vector<Uld> ULD_, int iterations_, int neighbourhoodSize_ = 50, int ignoreParameter_ = 50) : Solver(sorter_, merit_, boxes, ULD_), iterations(iterations_), originalUldList(ULD_), neighbourhoodSize(neighbourhoodSize_), ignoreParameter(ignoreParameter_)
    {
        /*ID based indexing of both score and boxMap
        Leaving buffer to prevent unwanted seg faults
        due to wrongly indexed IDs                 */
        insertionCounter.resize(boxes.size() + 5);
        score.resize(boxes.size() + 5);
        boxMap.resize(boxes.size() + 5);
    }

    
    /* Solver and assiting classes*/
    void solve() override;
    void arrangeDataFromIDVector(vector<int> idVector);
    void costDensityOptimize();


    /* Caching and Neighborhood Swap Related */
    bool isCached = false;
    int neighbourhoodSize = 50;
    int ignoreParameter = 50;

    void createCachedSolver(int cachingIndex);
    void solveCached(vector<Box> data);
    void insertedSwap(int _numSwaps);
    void bestSolutionSwaps(int swaps = 50, int ignoredObjects = 50, bool emptySort = false);

    /* DEPRECATED - Score Update Method related */
    void reinitialize(bool _swap, double k = 2.0, int num_swap = SWAP_PAIRS);
    void update_scores(int i);
    void optimize(int _iter);
};

int residueFunc(coords c, Box b, Solver *s);
