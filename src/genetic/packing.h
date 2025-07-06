#include "bits/stdc++.h"
#define For(i, n) for (int i = 0; i < n; i++)
#define FOR(k, i, n) for (int i = k; i < n; i++)
#define vi vector<int>
#define max(a, b) (a > b ? a : b)
#define maxP(a, b) (a.first > b.first ? a : b)
#define min(a, b) (a < b ? a : b)
#define INF 1000000000
#define pii pair<int, int>
#define NON_PRIORITY_COST 1000000
#define PRIORITY_ULD_COST 5000
#define RESIDUE_THRESHOLD 0
#define weightz 0.2
#define convertCoords(pt) pair<int, pair<int, pii>>(pt.box, pair<int, pii>(pt.x, pii(pt.y, pt.z)))
using namespace std;

bool check(const pair<pair<int, int>, pair<int, int>> &a, const pair<pair<int, int>, pair<int, int>> &b);
class Solver;
struct Box
{
    int l, b, h, weight, cost, ID;
    bool isPriority;
};
struct Sorter
{
    function<bool(Box, Box)> val;
};
struct coords
{
    int x, y, z, box;
};
struct Merit
{
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

    // Data Structures for Additional EP Creation
    function<bool(const int&, const int&)> compare_x;
    function<bool(const int&, const int&)> compare_y;   
    function<bool(const int&, const int&)> compare_z;

    vector<set<int, function<bool(const int&, const int&)>>> ULD_sorted_x;
    vector<set<int, function<bool(const int&, const int&)>>> ULD_sorted_y;
    vector<set<int, function<bool(const int&, const int&)>>> ULD_sorted_z;

    function<bool(const int&, const int&)> compare_x_base;
    function<bool(const int&, const int&)> compare_y_base;
    function<bool(const int&, const int&)> compare_z_base;
    
    set<int, function<bool(const int&, const int&)>> ULD_blocking_boxes_x;
    set<int, function<bool(const int&, const int&)>> ULD_blocking_boxes_y;
    set<int, function<bool(const int&, const int&)>> ULD_blocking_boxes_z;

    Solver(Sorter sorter_, Merit merit_, vector<Box> boxes, vector<Uld> ULD_);
    int cost();
    bool checkCollision(coords e, Box b);
    bool check_collision_2D(int x1_min, int x1_max, int y1_min, int y1_max, int x2_min, int x2_max, int y2_min, int y2_max);
    bool checkGravity(coords e, Box b);
    void gravity_pull(int i);
    void solve();
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
int residueFunc(coords c, Box b, Solver *s);
