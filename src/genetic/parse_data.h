#include <bits/stdc++.h>
#include "types.h"
#include "packing.h"

using namespace std;

PacketType ParsePacketType(const string &type);

optional<int> ParseCost(const string &cost);

void ParseULDs(const string &filename, vector<ULD> &ulds);

void ParsePackets(const string &filename, vector<Packet> &packets);

Uld convertULDToUld(ULD x);

Box convertPacketToBox(Packet x);
