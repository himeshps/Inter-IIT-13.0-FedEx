#include "parse_data.h"
using namespace std;

PacketType ParsePacketType(const string &type)
{
    return (type == "Priority") ? Priority : Economy;
}

optional<int> ParseCost(const string &cost)
{
    return (cost == "-") ? nullopt : optional<int>(stoi(cost));
}

void ParseULDs(const string &filename, vector<ULD> &ulds)
{
    ifstream input_file(filename);
    if (!input_file)
    {
        cerr << "Error opening ULD file: " << filename << endl;
        return;
    }

    string line;
    int uld_id = 1;
    while (getline(input_file, line))
    {
        stringstream line_stream(line);
        string identifier, length, width, height, weight;
        getline(line_stream, identifier, ',');
        getline(line_stream, length, ',');
        getline(line_stream, width, ',');
        getline(line_stream, height, ',');
        getline(line_stream, weight, ',');

        Dimensions dim = {stoi(length), stoi(width), stoi(height)};
        ulds.emplace_back(dim, uld_id++, stoi(weight));
    }

    input_file.close();
}

void ParsePackets(const string &filename, vector<Packet> &packets)
{
    ifstream input_file(filename);
    if (!input_file)
    {
        cerr << "Error opening Packet file: " << filename << endl;
        return;
    }

    string line;
    int package_id = 1;
    while (getline(input_file, line))
    {
        stringstream line_stream(line);
        string identifier, length, width, height, weight, type, cost;
        getline(line_stream, identifier, ',');
        getline(line_stream, length, ',');
        getline(line_stream, width, ',');
        getline(line_stream, height, ',');
        getline(line_stream, weight, ',');
        getline(line_stream, type, ',');
        getline(line_stream, cost, ',');
        Dimensions dim = {stoi(length), stoi(width), stoi(height)};
        PacketType packet_type = ParsePacketType(type);
        optional<int> delay_cost = ParseCost(cost);
        packets.emplace_back(dim, package_id++, stoi(weight), -1, packet_type, delay_cost);
    }
    input_file.close();
}

Uld convertULDToUld(ULD x)
{
    Uld r;
    r.dim.l = x.dimensions.length;
    r.dim.b = x.dimensions.width;
    r.dim.h = x.dimensions.height;
    r.weight = 0;
    r.maxWt = x.weight;
    r.com.x = r.dim.l / 2;
    r.com.x = r.dim.l / 2;
    r.com.x = r.dim.l / 2;
    r.ID = x.id;
    r.maxBound.x = r.maxBound.y = r.maxBound.z = 0;
    return r;
}

Box convertPacketToBox(Packet x)
{
    Box r;
    r.weight = x.weight;
    r.ID = x.id;
    r.cost = x.cost ? x.cost.value() : 0;
    r.isPriority = x.type == Priority;
    r.l = x.dimensions.length;
    r.b = x.dimensions.width;
    r.h = x.dimensions.height;
    return r;
}
