#include "genetic.h"

using namespace std;

int main()
{
    vector<Packet> packets;
    ParsePackets("packets.txt", packets);

    vector<struct ULD> ulds;
    ParseULDs("uld.txt", ulds);

    auto start = std::chrono::high_resolution_clock::now();

    Genetic gen(ulds, packets);
    gen.Execute();

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);

    std::cout << "Execution Time: " << duration.count() << " seconds" << std::endl;
    return 0;
}
