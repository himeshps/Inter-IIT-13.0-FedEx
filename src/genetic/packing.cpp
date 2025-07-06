#include "packing.h"

using namespace std;

bool check(const pair<pair<int, int>, pair<int, int>> &a, const pair<pair<int, int>, pair<int, int>> &b)
{
    int p = a.second.second;
    int q = b.second.second;
    // return true;
    return (p > q);
}
Solver::Solver(Sorter sorter_, Merit merit_, vector<Box> boxes, vector<Uld> ULD_)
{
    this->merit = merit_;
    this->sorter = sorter_;
    //        this->merit.init(this);
    //        this->sorter.init(this);
    this->data = boxes;
    def.x = def.y = def.z = def.box = -1;
    Box def_;
    def_.l = def_.b = def_.h = -1;
    this->placement.assign(boxes.size(), pair<coords, Box>(def, def_));
    ULDl = ULD_;
    ULDPackages.assign(ULDl.size(), set<int>());
    surfaces.assign(ULDl.size(), set<pair<int, pair<pair<int, int>, pair<int, int>>>>());
    ULDHasPriority.assign(ULDl.size(), false);

    compare_x = [this](const int &a, const int &b) { return placement[a].first.x + placement[a].second.l > placement[b].first.x + placement[b].second.l; };
    compare_y = [this](const int &a, const int &b) { return placement[a].first.y + placement[a].second.b > placement[b].first.y + placement[b].second.b; };
    compare_z = [this](const int &a, const int &b) { return placement[a].first.z + placement[a].second.h > placement[b].first.z + placement[b].second.h; };

    ULD_sorted_x.assign(ULDl.size(), set<int, function<bool(const int &, const int &)>>(compare_x));
    ULD_sorted_y.assign(ULDl.size(), set<int, function<bool(const int &, const int &)>>(compare_y));
    ULD_sorted_z.assign(ULDl.size(), set<int, function<bool(const int &, const int &)>>(compare_z));

    // sort blocking pkts in increasing order of base face coordinate
    compare_x_base = [this](const int &a, const int &b) { return placement[a].first.x < placement[b].first.x; }; 
    compare_y_base = [this](const int &a, const int &b) { return placement[a].first.y < placement[b].first.y; };
    compare_z_base = [this](const int &a, const int &b) { return placement[a].first.z < placement[b].first.z; };

    ULD_blocking_boxes_x = set<int, function<bool(const int &, const int &)>>(compare_x_base);
    ULD_blocking_boxes_y = set<int, function<bool(const int &, const int &)>>(compare_y_base);
    ULD_blocking_boxes_z = set<int, function<bool(const int &, const int &)>>(compare_z_base);
}

int Solver::cost()
{
    int c = 0;
    int count=0;
    set<int> priorityShipments;
    For(i, data.size())
    {
        if (placement[i].first.x == -1)
        {
            c -= data[i].cost;
            if (data[i].isPriority)
            {
                count++;
                c -= NON_PRIORITY_COST;
            }
        }
        else
        {
            //                c+=data[i].cost;

            if (data[i].isPriority)
                priorityShipments.insert(placement[i].first.box);
        }
    }
    c -= priorityShipments.size() * PRIORITY_ULD_COST;
    return c;
}
bool Solver::checkCollision(coords e, Box b)
{
    // checks collision of prespective packages with all other packages of sam eULD
    // if (e.x + b.l >= ULDl[e.box].dim.l or e.y + b.b >= ULDl[e.box].dim.b or e.z + b.h >= ULDl[e.box].dim.h or ULDl[e.box].weight + b.weight > ULDl[e.box].maxWt)
    if (e.x + b.l > ULDl[e.box].dim.l or e.y + b.b > ULDl[e.box].dim.b or e.z + b.h > ULDl[e.box].dim.h or ULDl[e.box].weight + b.weight > ULDl[e.box].maxWt){
        if(b.weight == 0){
            assert(0 == 1);
        }
        return true;
    }
    for (auto i : ULDPackages[e.box])
    {
        auto x = placement[i];
        if ((x.first.x < b.l + e.x and x.first.y < b.b + e.y and x.first.z < b.h + e.z and e.x < x.first.x + x.second.l and e.y < x.first.y + x.second.b and e.z < x.first.z + x.second.h))
        {
            return true;
        }
    }
    return false;
}

void Solver::solve()
{
    For(i, ULDl.size()) ep[pair<int, pair<int, pii>>(i, pair<int, pii>(0, pii(0, 0)))] = pair<int, pii>(ULDl[i].dim.l, pii(ULDl[i].dim.b, ULDl[i].dim.h));
    For(i, data.size())
    {
        Box b = data[i];
        pair<int, pair<coords, Box>> best;
        best.first = -INF;
        vector<Box> perms(6, b);
        perms[0].l = b.l;
        perms[0].b = b.b;
        perms[0].h = b.h; 
        perms[1].l = b.l;
        perms[1].h = b.b;
        perms[1].b = b.h;
        perms[2].l = b.b;
        perms[2].b = b.l;
        perms[2].h = b.h;
        perms[3].l = b.b;
        perms[3].b = b.h;
        perms[3].h = b.l;
        perms[4].l = b.h;
        perms[4].b = b.b;
        perms[4].h = b.l;
        perms[5].l = b.h;
        perms[5].b = b.l;
        perms[5].h = b.b;
        for (Box p : perms)
            for (auto x : ep)
            {
                // check if it can fit
                coords e;
                e.box = x.first.first;
                e.x = x.first.second.first;
                e.y = x.first.second.second.first;
                e.z = x.first.second.second.second;
                if(checkCollision(e,p))
                    continue;
                p.isPriority = b.isPriority;
                int score = this->merit.val(e, p, this);
                if (best.first < score){
                    
                    best = pair<int, pair<coords, Box>>(score, pair<coords, Box>(e, p));
                }
            }
        // update vals
        // cout << best.first << " " << best.second.first.x << " " << best.second.first.y << " " << best.second.first.z << " " << best.second.second.l << " " << best.second.second.b << " " << best.second.second.h << endl;
        if (best.first == -INF)
            continue;
        if (data[i].isPriority)
            ULDHasPriority[best.second.first.box] = true;
        // assert(best.second.second.weight == data[i].weight);
        placement[i] = best.second;
        // assert(placement[i].second.weight == data[i].weight);
        ULDPackages[best.second.first.box].insert(i);
        ULD_sorted_x[best.second.first.box].insert(i);
        ULD_sorted_y[best.second.first.box].insert(i);
        ULD_sorted_z[best.second.first.box].insert(i);
        
        gravity_pull(i);
        
        // addEP(i);
        addEP2(i);
        update(i);
        
    }
    // cout << c;
}

void Solver::update(int i)
{
    int b = placement[i].first.box;
    updateMaxBound(i);
    updateResidue(i);
    ULDl[b].com.x += (placement[i].first.x + placement[i].second.l / 2) * data[i].weight;
    ULDl[b].com.y += (placement[i].first.y + placement[i].second.b / 2) * data[i].weight;
    ULDl[b].com.z += (placement[i].first.z + placement[i].second.h / 2) * data[i].weight;
    ULDl[b].weight += data[i].weight;
    surfaces[b].insert(make_pair(placement[i].first.z + placement[i].second.h, make_pair(make_pair(placement[i].first.x, placement[i].first.y), make_pair(placement[i].first.x + placement[i].second.l, placement[i].first.y + placement[i].second.b))));
}

vector<coords> Solver::getCOM()
{
    vector<coords> r(ULDl.size());
    For(i, r.size())
    {
        if (ULDl[i].weight == 0)
        {
            r[i].x = ULDl[i].dim.l / 2;
            r[i].y = ULDl[i].dim.b / 2;
            r[i].z = ULDl[i].dim.h / 2;
            continue;
        }
        r[i].x = ULDl[i].com.x / ULDl[i].weight;
        r[i].y = ULDl[i].com.y / ULDl[i].weight;
        r[i].z = ULDl[i].com.z / ULDl[i].weight;
    }
    return r;
}

pair<int, pii> Solver::getResidueSpace(coords src)
{
    pair<int, pii> ret(beamprojectXPos(src).x - src.x, pii(beamprojectYPos(src).y - src.y, beamprojectZPos(src).z - src.z));
    return ret;
}
void Solver::initialise(coords &X, coords &ob, int x, int y, int z)
{
    X.x = ob.x + x;
    X.y = ob.y + y;
    X.z = ob.z + z;
    X.box = ob.box;
}

coords Solver::beamprojectZNeg(coords ob1)
{
    coords p, q, r;
    initialise(p, ob1, 1, 0, 0);
    initialise(q, ob1, 0, 1, 0);
    initialise(r, ob1, 1, 1, 0);
    coords p1 = rayProjectZNeg(p);
    coords q1 = rayProjectZNeg(ob1);
    coords r1 = rayProjectZNeg(q);
    coords s1 = rayProjectZNeg(r);
    coords ans;
    ans.x = ob1.x;
    ans.y = ob1.y;
    ans.z = min(min(min(p1.z, q1.z), r1.z), s1.z);
    ans.box = ob1.box;
    return ans;
}

coords Solver::beamprojectYNeg(coords ob1)
{
    coords p, q, r;
    initialise(p, ob1, 1, 0, 0);
    initialise(q, ob1, 0, 0, 1);
    initialise(r, ob1, 1, 0, 1);
    coords p1 = rayProjectYNeg(p);
    coords q1 = rayProjectYNeg(ob1);
    coords r1 = rayProjectYNeg(q);
    coords s1 = rayProjectYNeg(r);
    coords ans;
    ans.x = ob1.x;
    ans.y = min(min(min(p1.y, q1.y), r1.y), s1.y);
    ans.z = ob1.z;
    ans.box = ob1.box;
    return ans;
}
coords Solver::beamprojectXNeg(coords ob1)
{
    coords p, q, r;
    initialise(p, ob1, 0, 1, 0);
    initialise(q, ob1, 0, 0, 1);
    initialise(r, ob1, 0, 1, 1);
    coords p1 = rayProjectXNeg(p);
    coords q1 = rayProjectXNeg(ob1);
    coords r1 = rayProjectXNeg(q);
    coords s1 = rayProjectXNeg(r);
    coords ans;
    ans.x = min(min(min(p1.x, q1.x), r1.x), s1.x);
    ans.y = ob1.y;
    ans.z = ob1.z;
    ans.box = ob1.box;
    return ans;
}
coords Solver::beamprojectZPos(coords ob1)
{
    coords p, q, r;
    initialise(p, ob1, 1, 0, 0);
    initialise(q, ob1, 0, 1, 0);
    initialise(r, ob1, 1, 1, 0);
    coords p1 = rayProjectZPos(p);
    coords q1 = rayProjectZPos(ob1);
    coords r1 = rayProjectZPos(q);
    coords s1 = rayProjectZPos(r);
    coords ans;
    ans.x = ob1.x;
    ans.y = ob1.y;
    ans.z = min(min(min(p1.z, q1.z), r1.z), s1.z);
    ans.box = ob1.box;
    return ans;
}
coords Solver::beamprojectYPos(coords ob1)
{
    coords p, q, r;
    initialise(p, ob1, 1, 0, 0);
    initialise(q, ob1, 0, 0, 1);
    initialise(r, ob1, 1, 0, 1);
    coords p1 = rayProjectYPos(p);
    coords q1 = rayProjectYPos(ob1);
    coords r1 = rayProjectYPos(q);
    coords s1 = rayProjectYPos(r);
    coords ans;
    ans.x = ob1.x;
    ans.y = min(min(min(p1.y, q1.y), r1.y), s1.y);
    ans.z = ob1.z;
    ans.box = ob1.box;
    return ans;
}
coords Solver::beamprojectXPos(coords ob1)
{
    coords p, q, r;
    initialise(p, ob1, 0, 1, 0);
    initialise(q, ob1, 0, 0, 1);
    initialise(r, ob1, 0, 1, 1);
    coords p1 = rayProjectXPos(p);
    coords q1 = rayProjectXPos(ob1);
    coords r1 = rayProjectXPos(q);
    coords s1 = rayProjectXPos(r);
    coords ans;
    ans.x = min(min(min(p1.x, q1.x), r1.x), s1.x);
    ans.y = ob1.y;
    ans.z = ob1.z;
    ans.box = ob1.box;
    return ans;
}

// Older EP2 Implementation

// void Solver::addEP2(int i)
// {
//     // ep.erase(pair<int,pii>(placement[i].first.x,pii(placement[i].first.y,placement[i].first.z)));
//     coords ob1;
//     auto p = placement[i];
//     // placement[i].first=def;
//     ob1.x = p.first.x + placement[i].second.l;
//     ob1.y = p.first.y;
//     ob1.z = p.first.z + p.second.h;
//     vector<pair<pair<int, int>, pair<int, int>>> faces, faces_checked;
//     For(j, i)
//     {
//         faces.push_back(make_pair(make_pair(placement[j].first.x, placement[j].first.y), make_pair(placement[j].first.y + placement[j].second.b, placement[j].first.z + placement[j].second.h)));
//     }
//     sort(faces.begin(), faces.end(), check);
//     For(j, i)
//     {
//         int x1 = faces[j].first.first;
//         // int y1=faces[j].second.first;
//         int z1 = faces[j].second.second;
//         if (ob1.z < z1 || ob1.x > x1)
//             continue;
//         if (((ob1.y >= faces[j].first.second) && (ob1.y <= faces[j].second.first)) == false)
//             continue;
//         int t = 0;
//         for (auto yy : placement)
//         {
//             int x = yy.first.x;
//             int y = yy.first.y;
//             int z = yy.first.z;
//             if (ob1.x < x && x < x1 && z1 > yy.first.z && z1 < yy.first.z + yy.second.h)
//                 t = 1;
//         }
//         // if(t==0)
//         if (t == 0)
//         {
//             coords d;
//             d.x = ob1.x;
//             d.y = ob1.y;
//             d.z = faces[j].second.second;
//             auto r = getResidueSpace(d);
//             if (r.first * r.second.first * r.second.second != 0)
//                 ep[convertCoords(d)] = r;
//         }
//         // faces_checked.push_back(faces[j]);
//     }
// }
void Solver::projection_neg_z_advanced(vector<coords> &advanced_eps, coords start, int self_pid){

    bool hit_bottom = true;
    for(auto i: ULD_sorted_z[start.box])
    {
        coords potential_ep;
        potential_ep.box = start.box;
        potential_ep.x = start.x;
        potential_ep.y = start.y;
        potential_ep.z = placement[i].first.z + placement[i].second.h;

        if(i == self_pid)
            continue;
        auto pkt = placement[i];
        if(pkt.first.x == -1) continue;
        if(pkt.first.x + pkt.second.l <= start.x) continue;
        if(pkt.first.y + pkt.second.b <= start.y) continue;
        if(pkt.first.z >= start.z) continue;

        // case 0: ray hits the object (no more projections)
        if((start.x >= pkt.first.x) && (start.x < pkt.first.x + pkt.second.l) &&
           (start.y >= pkt.first.y) && (start.y < pkt.first.y + pkt.second.b)) {
            advanced_eps.push_back(potential_ep);
            hit_bottom = false;
            break;
        }
        // check if the pkt is being blocked by other packets
        if(start.x >= pkt.first.x){
            bool is_blocked = false;
            for(auto j: ULD_blocking_boxes_z){
                auto blocking_pkt = placement[j];
                if(blocking_pkt.first.x == -1) continue; // although this should not happen
                if(blocking_pkt.first.z > pkt.first.z + pkt.second.h) continue;
                if(!((blocking_pkt.first.y + blocking_pkt.second.b > start.y) && (blocking_pkt.first.y <= pkt.first.y))) continue;
                if(!((start.x >= blocking_pkt.first.x) && (start.x < blocking_pkt.first.x + blocking_pkt.second.l))) continue;
                is_blocked = true;
                break;
            }
            if(is_blocked) continue;
        }
        else if(start.y >= pkt.first.y){
            // give line by line 
            bool is_blocked = false;
            for(auto j: ULD_blocking_boxes_z){
                auto blocking_pkt = placement[j];
                if(blocking_pkt.first.x == -1) continue; // although this should not happen
                if(blocking_pkt.first.z > pkt.first.z + pkt.second.h) continue;
                if(!((blocking_pkt.first.x + blocking_pkt.second.l > start.x) && (blocking_pkt.first.x <= pkt.first.x))) continue;
                if(!((start.y >= blocking_pkt.first.y) && (start.y < blocking_pkt.first.y + blocking_pkt.second.b))) continue;
                is_blocked = true;
                break;
            }
            if(is_blocked) continue;   
        }
        else{ // diagonal intersection
            bool is_blocked = false;
            for(auto j: ULD_blocking_boxes_z){
                auto blocking_pkt = placement[j];
                if(blocking_pkt.first.x == -1) continue; // although this should not happen
                if(blocking_pkt.first.z > pkt.first.z + pkt.second.h) continue;
                is_blocked = check_collision_2D(pkt.first.x, pkt.first.x + pkt.second.l, 
                                                pkt.first.y, pkt.first.y + pkt.second.b,
                                                blocking_pkt.first.x, blocking_pkt.first.x + blocking_pkt.second.l, 
                                                blocking_pkt.first.y, blocking_pkt.first.y + blocking_pkt.second.b);
                if(is_blocked) break;
            }
            if(is_blocked) continue;   
        }
        
        // add to the blocking pkts list
        ULD_blocking_boxes_z.insert(i);
        if(pkt.first.z + pkt.second.h < start.z){
            advanced_eps.push_back(potential_ep);
        }
    }
    ULD_blocking_boxes_z.clear();
    if(hit_bottom){
        coords potential_ep;
        potential_ep.box = start.box;
        potential_ep.x = start.x;
        potential_ep.y = start.y;
        potential_ep.z = 0;
        advanced_eps.push_back(potential_ep);
    }
    return;

}

void Solver::projection_neg_y_advanced(vector<coords> &advanced_eps, coords start, int self_pid){
    
        bool hit_bottom = true;
        for(auto i: ULD_sorted_y[start.box])
        {
            coords potential_ep;
            potential_ep.box = start.box;
            potential_ep.x = start.x;
            potential_ep.y = placement[i].first.y + placement[i].second.b;
            potential_ep.z = start.z;
    
            if(i == self_pid)
                continue;
            auto pkt = placement[i];
            if(pkt.first.x == -1) continue;
            if(pkt.first.x + pkt.second.l <= start.x) continue;
            if(pkt.first.z + pkt.second.h <= start.z) continue;
            if(pkt.first.y >= start.y) continue;
    
            // case 0: ray hits the object (no more projections)
            if((start.x >= pkt.first.x) && (start.x < pkt.first.x + pkt.second.l) &&
               (start.z >= pkt.first.z) && (start.z < pkt.first.z + pkt.second.h)) {
                advanced_eps.push_back(potential_ep);
                hit_bottom = false;
                break;
            }
            // check if the pkt is being blocked by other packets
            if(start.x >= pkt.first.x){
                bool is_blocked = false;
                for(auto j: ULD_blocking_boxes_y){
                    auto blocking_pkt = placement[j];
                    if(blocking_pkt.first.x == -1) continue; // although this should not happen
                    if(blocking_pkt.first.y > pkt.first.y + pkt.second.b) continue;
                    if(!((blocking_pkt.first.z + blocking_pkt.second.h > start.z) && (blocking_pkt.first.z <= pkt.first.z))) continue;
                    if(!((start.x >= blocking_pkt.first.x) && (start.x < blocking_pkt.first.x + blocking_pkt.second.l))) continue;
                    is_blocked = true;
                    break;
                }
                if(is_blocked) continue;
            }
            else if(start.z >= pkt.first.z){
                // give line by line 
                bool is_blocked = false;
                for(auto j: ULD_blocking_boxes_y){
                    auto blocking_pkt = placement[j];
                    if(blocking_pkt.first.x == -1) continue; // although this should not happen
                    if(blocking_pkt.first.y > pkt.first.y + pkt.second.b) continue;
                    if(!((blocking_pkt.first.x + blocking_pkt.second.l > start.x) && (blocking_pkt.first.x <= pkt.first.x))) continue;
                    if(!((start.z >= blocking_pkt.first.z) && (start.z < blocking_pkt.first.z + blocking_pkt.second.h))) continue;
                    is_blocked = true;
                    break;
                }
                if(is_blocked) continue;   
            }

            else{ // diagonal intersection
                bool is_blocked = false;
                for(auto j: ULD_blocking_boxes_y){
                    auto blocking_pkt = placement[j];
                    if(blocking_pkt.first.x == -1) continue; // although this should not happen
                    if(blocking_pkt.first.y > pkt.first.y + pkt.second.b) continue;
                    is_blocked = check_collision_2D(pkt.first.x, pkt.first.x + pkt.second.l, 
                                                    pkt.first.z, pkt.first.z + pkt.second.h,
                                                    blocking_pkt.first.x, blocking_pkt.first.x + blocking_pkt.second.l, 
                                                    blocking_pkt.first.z, blocking_pkt.first.z + blocking_pkt.second.h);
                    if(is_blocked) break;
                }
                if(is_blocked) continue;   
            }

            // add to the blocking pkts list
            ULD_blocking_boxes_y.insert(i);
            if(pkt.first.y + pkt.second.b < start.y){
                advanced_eps.push_back(potential_ep);
            }
        }
        ULD_blocking_boxes_y.clear();
        if(hit_bottom){
            coords potential_ep;
            potential_ep.box = start.box;
            potential_ep.x = start.x;
            potential_ep.y = 0;
            potential_ep.z = start.z;
            advanced_eps.push_back(potential_ep);
        }
        return;
}
bool Solver::check_collision_2D(int x1_min, int x1_max, int y1_min, int y1_max, int x2_min, int x2_max, int y2_min, int y2_max)
{
    bool is_x2_min_inside = (x2_min >= x1_min) && (x2_min <= x1_max);
    bool is_x2_max_inside = (x2_max > x1_min) && (x2_max <= x1_max);
    bool is_y2_min_inside = (y2_min >= y1_min) && (y2_min <= y1_max);
    bool is_y2_max_inside = (y2_max > y1_min) && (y2_max <= y1_max);
    return (is_x2_min_inside || is_x2_max_inside) && (is_y2_min_inside || is_y2_max_inside);
}
void Solver::projection_neg_x_advanced(vector<coords> &advanced_eps, coords start, int self_pid){
    
        bool hit_bottom = true;
        for(auto i: ULD_sorted_x[start.box])
        {
            coords potential_ep;
            potential_ep.box = start.box;
            potential_ep.x = placement[i].first.x + placement[i].second.l;
            potential_ep.y = start.y;
            potential_ep.z = start.z;
    
            if(i == self_pid)
                continue;
            auto pkt = placement[i];
            if(pkt.first.x == -1) continue;
            if(pkt.first.y + pkt.second.b <= start.y) continue;
            if(pkt.first.z + pkt.second.h <= start.z) continue;
            if(pkt.first.x >= start.x) continue;
    
            // case 0: ray hits the object (no more projections)
            if((start.y >= pkt.first.y) && (start.y < pkt.first.y + pkt.second.b) &&
               (start.z >= pkt.first.z) && (start.z < pkt.first.z + pkt.second.h)) {
                advanced_eps.push_back(potential_ep);
                hit_bottom = false;
                break;
            }
            // check if the pkt is being blocked by other packets
            if(start.y >= pkt.first.y){
                bool is_blocked = false;
                for(auto j: ULD_blocking_boxes_x){
                    auto blocking_pkt = placement[j];
                    if(blocking_pkt.first.y == -1) continue; // although this should not happen
                    if(blocking_pkt.first.x > pkt.first.x + pkt.second.l) continue;
                    if(!((blocking_pkt.first.z + blocking_pkt.second.h > start.z) && (blocking_pkt.first.z <= pkt.first.z))) continue;
                    if(!((start.y >= blocking_pkt.first.y) && (start.y < pkt.first.y + pkt.second.b))) continue;
                    is_blocked = true;
                    break;
                }
                if(is_blocked) continue;
            }
            else if(start.z >= pkt.first.z){
                // give line by line 
                bool is_blocked = false;
                for(auto j: ULD_blocking_boxes_x){
                    auto blocking_pkt = placement[j];
                    if(blocking_pkt.first.y == -1) continue; // although this should not happen
                    if(blocking_pkt.first.x > pkt.first.x + pkt.second.l) continue;
                    if(!((blocking_pkt.first.y + blocking_pkt.second.b > start.y) && (blocking_pkt.first.y <= pkt.first.y))) continue;
                    if(!((start.z >= blocking_pkt.first.z) && (start.z < blocking_pkt.first.z + blocking_pkt.second.h))) continue;
                    is_blocked = true;
                    break;
                }
                if(is_blocked) continue;   
            }
            else{ // diagonal intersection
                bool is_blocked = false;
                for(auto j: ULD_blocking_boxes_x){
                    auto blocking_pkt = placement[j];
                    if(blocking_pkt.first.y == -1) continue; // although this should not happen
                    if(blocking_pkt.first.x > pkt.first.x + pkt.second.l) continue;
                    is_blocked = check_collision_2D(pkt.first.y, pkt.first.y + pkt.second.b, 
                                                    pkt.first.z, pkt.first.z + pkt.second.h,
                                                    blocking_pkt.first.y, blocking_pkt.first.y + blocking_pkt.second.b, 
                                                    blocking_pkt.first.z, blocking_pkt.first.z + blocking_pkt.second.h);
                    if(is_blocked) break;
                }
                if(is_blocked) continue;   
            }

            // add to the blocking pkts list
            ULD_blocking_boxes_x.insert(i);
            if(pkt.first.x + pkt.second.l < start.x){
                advanced_eps.push_back(potential_ep);
            }
        }
        ULD_blocking_boxes_x.clear();
        if(hit_bottom){
            coords potential_ep;
            potential_ep.box = start.box;
            potential_ep.x = 0;
            potential_ep.y = start.y;
            potential_ep.z = start.z;
            advanced_eps.push_back(potential_ep);
        }
        return;
}
void Solver::addEP2(int i)
{
    coords ob1, ob2, ob3;
    auto p = placement[i];
    
    int uid = p.first.box;
    ob1.box = uid;
    ob1.x = p.first.x;
    ob1.y = p.first.y + p.second.b;
    ob1.z = p.first.z + p.second.h;

    ob2.box = uid;
    ob2.x = p.first.x + p.second.l;
    ob2.y = p.first.y;
    ob2.z = p.first.z + p.second.h;

    ob3.box = uid;
    ob3.x = p.first.x + p.second.l;
    ob3.y = p.first.y + p.second.b;
    ob3.z = p.first.z;
    
    
    vector<coords> advanced_eps;
    projection_neg_y_advanced(advanced_eps, ob1, i);
    projection_neg_z_advanced(advanced_eps, ob1, i);

    projection_neg_x_advanced(advanced_eps, ob2, i);
    projection_neg_z_advanced(advanced_eps, ob2, i);

    projection_neg_x_advanced(advanced_eps, ob3, i);
    projection_neg_y_advanced(advanced_eps, ob3, i);

    for(auto x: advanced_eps){
        auto r = getResidueSpace(x);
        if((r.first > RESIDUE_THRESHOLD) and (r.second.first > RESIDUE_THRESHOLD) and (r.second.second) > RESIDUE_THRESHOLD)
            ep[convertCoords(x)] = r;
    }
    placement[i] = p; 
}


void Solver::addEP(int i)
{
    // ep.erase(pair<int, pair<int, pii>>(placement[i].first.box, pair<int, pii>(placement[i].first.x, pii(placement[i].first.y, placement[i].first.z))));
    coords ob1;
    auto p = placement[i];
    placement[i].first = def;
    ob1.box = p.first.box;
    ob1.x = p.first.x;
    ob1.y = p.first.y + placement[i].second.b;
    ob1.z = p.first.z + p.second.h;
    auto t = beamprojectYNeg(ob1);
    auto r = getResidueSpace(t);
    if (r.first > RESIDUE_THRESHOLD and r.second.first > RESIDUE_THRESHOLD and r.second.second > RESIDUE_THRESHOLD)
        ep[convertCoords(t)] = r;
    t = beamprojectZNeg(ob1);
    r = getResidueSpace(t);
    if (r.first > RESIDUE_THRESHOLD and r.second.first > RESIDUE_THRESHOLD and r.second.second > RESIDUE_THRESHOLD)
        ep[convertCoords(t)] = r;
    coords ob2;
    ob2.box = p.first.box;
    ob2.x = p.first.x + p.second.l;
    ob2.y = p.first.y;
    ob2.z = p.first.z + p.second.h;
    t = beamprojectXNeg(ob2);
    r = getResidueSpace(t);
    if (r.first > RESIDUE_THRESHOLD and r.second.first > RESIDUE_THRESHOLD and r.second.second > RESIDUE_THRESHOLD)
        ep[convertCoords(t)] = r;
    t = beamprojectZNeg(ob2);
    r = getResidueSpace(t);
    if (r.first > RESIDUE_THRESHOLD and r.second.first > RESIDUE_THRESHOLD and r.second.second > RESIDUE_THRESHOLD)
        ep[convertCoords(t)] = r;
    coords ob3;
    ob3.box = p.first.box;
    ob3.x = p.first.x + p.second.l;
    ob3.y = p.first.y + p.second.b;
    ob3.z = p.first.z;
    t = beamprojectXNeg(ob3);
    r = getResidueSpace(t);
    if (r.first > RESIDUE_THRESHOLD and r.second.first > RESIDUE_THRESHOLD and r.second.second >= RESIDUE_THRESHOLD)
        ep[convertCoords(t)] = r;
    t = beamprojectYNeg(ob3);
    r = getResidueSpace(t);
    if (r.first > RESIDUE_THRESHOLD and r.second.first > RESIDUE_THRESHOLD and r.second.second > RESIDUE_THRESHOLD)
        ep[convertCoords(t)] = r;
    placement[i] = p;
}
void Solver::updateMaxBound(int i)
{
    int b = placement[i].first.box;
    ULDl[b].maxBound.x = max(ULDl[b].maxBound.x, placement[i].first.x + placement[i].second.l);
    ULDl[b].maxBound.y = max(ULDl[b].maxBound.y, placement[i].first.y + placement[i].second.b);
    ULDl[b].maxBound.z = max(ULDl[b].maxBound.z, placement[i].first.z + placement[i].second.h);
}
void Solver::updateResidue(int i)
{
    // update residual space also to be done
    auto it = ep.lower_bound(convertCoords(placement[i].first));
    int b = placement[i].first.box;
    while (it != ep.end() and it->first.first == b)
    {
        auto p = *it;
        coords e;
        e.box = p.first.first;
        e.x = p.first.second.first;
        e.y = p.first.second.second.first;
        e.z = p.first.second.second.second;
        int t = XBeamIntersectionWithBox(e, i);
        if (t != -1)
            p.second.first = min(p.second.first, t);
        t = YBeamIntersectionWithBox(e, i);
        if (t != -1)
            p.second.second.first = min(p.second.second.first, t);
        t = ZBeamIntersectionWithBox(e, i);
        if (t != -1)
            p.second.second.second = min(p.second.second.second, t);
        it++;
    }
}
int Solver::XBeamIntersectionWithBox(coords start, int ind)
{
    int r = INF;
    int b = start.box;
    For(i, 2) For(j, 2)
    {
        auto p = start;
        p.y = min(p.y + i, ULDl[b].dim.b - 1);
        p.z = min(p.z + j, ULDl[b].dim.h - 1);
        int t = XRayIntersectionWithBox(p, ind);
        if (t != -1)
        {
            r = min(r, t);
        }
    }
    return r == INF ? -1 : r;
}
int Solver::YBeamIntersectionWithBox(coords start, int ind)
{
    int r = INF;
    int b = start.box;
    For(i, 2) For(j, 2)
    {
        auto p = start;
        p.x = min(p.x + i, ULDl[b].dim.l - 1);
        p.z = min(p.z + j, ULDl[b].dim.h - 1);
        int t = YRayIntersectionWithBox(p, ind);
        if (t != -1)
        {
            r = min(r, t);
        }
    }
    return r == INF ? -1 : r;
}
int Solver::ZBeamIntersectionWithBox(coords start, int ind)
{
    int r = INF;
    int b = start.box;
    For(i, 2) For(j, 2)
    {
        auto p = start;
        p.x = min(p.x + i, ULDl[b].dim.l - 1);
        p.y = min(p.y + j, ULDl[b].dim.b - 1);
        int t = ZRayIntersectionWithBox(p, ind);
        if (t != -1)
        {
            r = min(r, t);
        }
    }
    return r == INF ? -1 : r;
}
int Solver::XRayIntersectionWithBox(coords start, int ind)
{
    if (start.y < placement[ind].first.y + placement[ind].second.b and start.y > placement[ind].first.y and start.z < placement[ind].first.z + placement[ind].second.h and start.z > placement[ind].first.z and start.x < placement[ind].first.x + placement[ind].second.l and start.box == placement[ind].first.box)
        return placement[ind].first.x;
    return -1;
}
int Solver::YRayIntersectionWithBox(coords start, int ind)
{
    if (start.x < placement[ind].first.x + placement[ind].second.l and start.x > placement[ind].first.x and start.z < placement[ind].first.z + placement[ind].second.h and start.z > placement[ind].first.z and start.y < placement[ind].first.y + placement[ind].second.b and start.box == placement[ind].first.box)
        return placement[ind].first.y;
    return -1;
}
int Solver::ZRayIntersectionWithBox(coords start, int ind)
{
    if (start.x < placement[ind].first.x + placement[ind].second.l and start.x > placement[ind].first.x and start.y < placement[ind].first.y + placement[ind].second.b and start.y > placement[ind].first.y and start.z < placement[ind].first.z + placement[ind].second.h and start.box == placement[ind].first.box)
        return placement[ind].first.z;
    return -1;
}
coords Solver::rayProjectXNeg(coords start)
{
    int x = 0;
    for (int i : ULDPackages[start.box])
    {
        if (placement[i].first.x != -1 and start.y <= placement[i].first.y + placement[i].second.b and start.y >= placement[i].first.y and start.z <= placement[i].first.z + placement[i].second.h and start.z >= placement[i].first.z and start.x >= placement[i].first.x)
            x = max(x, placement[i].first.x + placement[i].second.l);
    }
    start.x = min(start.x, x);
    return start;
}
coords Solver::rayProjectYNeg(coords start)
{
    int x = 0;
    for (int i : ULDPackages[start.box])
    {
        if (placement[i].first.x != -1 and start.x <= placement[i].first.x + placement[i].second.l and start.x >= placement[i].first.x and start.z <= placement[i].first.z + placement[i].second.h and start.z >= placement[i].first.z and start.y >= placement[i].first.y)
            x = max(x, placement[i].first.y + placement[i].second.b);
    }
    start.y = min(start.y, x);
    return start;
}
coords Solver::rayProjectZNeg(coords start)
{
    int x = 0;
    for (int i : ULDPackages[start.box])
    {
        if (placement[i].first.x != -1 and start.x <= placement[i].first.x + placement[i].second.l and start.x >= placement[i].first.x and start.y <= placement[i].first.y + placement[i].second.b and start.y >= placement[i].first.y and start.z >= placement[i].first.z)
            x = max(x, placement[i].first.z + placement[i].second.h);
    }
    start.z = min(start.z, x);
    return start;
}
coords Solver::rayProjectXPos(coords start)
{
    int x = ULDl[start.box].dim.l;
    for (int i : ULDPackages[start.box])
    {
        if (placement[i].first.x != -1 and start.y < placement[i].first.y + placement[i].second.b and start.y > placement[i].first.y and start.z < placement[i].first.z + placement[i].second.h and start.z > placement[i].first.z and start.x < placement[i].first.x + placement[i].second.l)
            x = min(x, placement[i].first.x);
    }
    start.x = max(start.x, x);
    return start;
}
coords Solver::rayProjectYPos(coords start)
{
    int x = ULDl[start.box].dim.b;
    for (int i : ULDPackages[start.box])
    {
        if (placement[i].first.x != -1 and start.x < placement[i].first.x + placement[i].second.l and start.x > placement[i].first.x and start.z < placement[i].first.z + placement[i].second.h and start.z > placement[i].first.z and start.y < placement[i].first.y + placement[i].second.b)
            x = min(x, placement[i].first.y);
    }
    start.y = max(start.y, x);
    return start;
}
coords Solver::rayProjectZPos(coords start)
{
    int x = ULDl[start.box].dim.h;
    for (int i : ULDPackages[start.box])
    {
        if (placement[i].first.x != -1 and start.x < placement[i].first.x + placement[i].second.l and start.x > placement[i].first.x and start.y < placement[i].first.y + placement[i].second.b and start.y > placement[i].first.y and start.z < placement[i].first.z + placement[i].second.h)
            x = min(x, placement[i].first.z);
    }
    start.z = max(start.z, x);
    return start;
}
bool Solver::checkGravity(coords e, Box b)
{
    // cout << surfaces.size();
    if (e.z == 0)
        return false;
    // return false;
    // sort(surfaces.begin(),surfaces.end());
    // auto start=surfaces.lower_bound(make_pair(e.z,make_pair(make_pair(0,0),make_pair(0,0))));
    // auto end=surfaces.lower_bound(make_pair(e.z,make_pair(make_pair(3000,3000),make_pair(3000,3000))));
    // for(auto x:surfaces)
    // cout << x.first << " ";
    // cout << "\n";
    pair<int, int> x1_min = make_pair(e.x, e.y), x1_max = {e.x + b.l, e.y + b.b};
    int flag = 0;
    int c = 0;
    for (auto i : surfaces[e.box])
    {
        if ((i).first != e.z)
            continue;
        c++;
    }
    if (c == 0)
        return true;
    for (auto i : surfaces[e.box])
    {
        if (i.first != e.z)
            continue;
        pair<int, pair<pair<int, int>, pair<int, int>>> p = i;
        pair<int, int> x2_min = make_pair(p.second.first.first, p.second.first.second), x2_max = {p.second.second.first, p.second.second.second};
        if (x1_max.first > x2_min.first && x1_min.first < x2_max.first && x1_max.second > x2_min.second && x1_min.second < x2_max.second)
            flag = 1;
    }
    if (flag == 1)
        return false;
    return true;
}
void Solver::gravity_pull(int i)
{
    pair<int, int> x1_min = make_pair(placement[i].first.x, placement[i].first.y), x1_max = {placement[i].first.x + placement[i].second.l, placement[i].first.y + placement[i].second.b};
    int m = 0;
    for (auto x : surfaces[placement[i].first.box])
    {
        pair<int, pair<pair<int, int>, pair<int, int>>> p = x;
        pair<int, int> x2_min = make_pair(p.second.first.first, p.second.first.second), x2_max = {p.second.second.first, p.second.second.second};
        if (x1_max.first > x2_min.first && x1_min.first < x2_max.first && x1_max.second > x2_min.second && x1_min.second < x2_max.second && x.first <= placement[i].first.z)
            m = max(m, x.first);
    }
    placement[i].first.z = m;
}
// void Solver::gravity_pull(int i)
// {
//     auto x1_min = make_pair(placement[i].first.x, placement[i].first.y);
//     auto x1_max = make_pair(placement[i].first.x + placement[i].second.l, placement[i].first.y + placement[i].second.b);

//     int m = 0;
//     for (auto x : surfaces)
//     {
//         auto p = x;
//         auto x2_min = make_pair(p.second.first.first, p.second.first.second);
//         auto x2_max = make_pair(p.second.second.first, p.second.second.second);

//         bool is_overlapping = x1_max.first > x2_min.first &&
//                               x1_min.first < x2_max.first &&
//                               x1_max.second > x2_min.second &&
//                               x1_min.second < x2_max.second;

//         if (is_overlapping && p.first <= placement[i].first.z)
//         {
//             m = max(m, p.first);
//         }
//     }

//     placement[i].first.z = m;
//     // c+=1-checkGravity(placement[i].first,placement[i].second);
// }

int residueFunc(coords c, Box b, Solver *s)
{
    int r = 0;
    r += (1LL * (s->ULDHasPriority[c.box]) * 100000000LL) * b.isPriority;
    float relativeDifference = (s->ep[convertCoords(c)].first - b.l) / 1.0 / s->ep[convertCoords(c)].first + (s->ep[convertCoords(c)].second.first - b.b) / 1.0 / s->ep[convertCoords(c)].second.first + weightz * (s->ep[convertCoords(c)].second.second - b.h) / 1.0 / s->ep[convertCoords(c)].second.second;
    relativeDifference *= 1000000;
    //    float relativeDifference =(s->ep[convertCoords(c)].first - b.l)+(s->ep[convertCoords(c)].second.first - b.b)+0.1*(s->ep[convertCoords(c)].second.second - b.h);
    r += relativeDifference;
    return r;
}
