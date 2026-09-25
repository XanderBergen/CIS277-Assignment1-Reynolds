#include <iostream>
#include <iomanip>
#include <cstring>
#include <vector>

#include "MemoryPool.h"

int main()
{
    const std::size_t blockSize = 512;
    const std::size_t blockCount = 8;

    MemoryPool pool(blockSize, blockCount);

    std::cout << "Network Packet Buffer Pool\n\n";

    std::cout << "Block Size:       "
        << pool.blockSize() << " bytes\n";

    std::cout << "Blocks:           "
        << blockCount << '\n';

    std::cout << "Total Capacity:   "
        << pool.capacity() << " bytes\n\n";


    // Allocate several blocks

    void* packet1 = pool.allocate();
    void* packet2 = pool.allocate();
    void* packet3 = pool.allocate();

    std::cout << "Packet 1 allocated: " << packet1 << '\n';
    std::cout << "Packet 2 allocated: " << packet2 << '\n';
    std::cout << "Packet 3 allocated: " << packet3 << '\n';

    std::cout << "\nAvailable blocks: "
        << pool.availableBlocks() << '\n';

    std::cout << "Allocated blocks: "
        << pool.allocatedBlocks() << "\n\n";


    // Store binary packet data

    unsigned char packetData[] =
    {
        0x45, 0x00, 0x00, 0x3C,
        0xAB, 0xCD, 0x12, 0x34
    };

    if (packet1 != nullptr &&
        sizeof(packetData) <= pool.blockSize())
    {
        std::memcpy(packet1, packetData, sizeof(packetData));

        std::cout << "Binary packet written to Packet 1.\n";

        unsigned char* storedData =
            static_cast<unsigned char*>(packet1);

        std::cout << "Packet 1 data: ";

        for (std::size_t i = 0; i < sizeof(packetData); ++i)
        {
            std::cout << "0x"
                << std::hex
                << std::uppercase
                << std::setw(2)
                << std::setfill('0')
                << static_cast<int>(storedData[i])
                << ' ';
        }

        std::cout << std::dec
            << std::nouppercase
            << std::setfill(' ')
            << "\n";

        bool dataMatches =
            std::memcmp(packet1,
                packetData,
                sizeof(packetData)) == 0;

        std::cout << "Binary data read-back verification: "
            << (dataMatches ? "successful" : "failed")
            << "\n\n";
    }


    // Release a block

    void* releasedAddress = packet2;

    if (pool.deallocate(packet2))
    {
        std::cout << "Packet 2 released.\n";
    }

    std::cout << "Available blocks: "
        << pool.availableBlocks() << '\n';

    std::cout << "Allocated blocks: "
        << pool.allocatedBlocks() << "\n\n";


    // Demonstrate memory reuse

    void* packet4 = pool.allocate();

    std::cout << "Packet 4 allocated: "
        << packet4 << '\n';

    if (packet4 == releasedAddress)
    {
        std::cout
            << "Packet 4 reused the previously released block.\n";
    }
    else
    {
        std::cout
            << "Packet 4 did not reuse the released block.\n";
    }


    // Exhaust the memory pool

    std::cout << "\nAttempting to exhaust pool...\n";

    std::vector<void*> extraBlocks;

    while (true)
    {
        void* block = pool.allocate();

        if (block == nullptr)
        {
            break;
        }

        extraBlocks.push_back(block);

        std::cout << "Additional block allocated: "
            << block << '\n';
    }


    // Attempt allocation after exhaustion

    void* failedAllocation = pool.allocate();

    if (failedAllocation == nullptr)
    {
        std::cout << "\nNo blocks available.\n";
        std::cout << "allocate() returned nullptr.\n";
    }

    std::cout << "\nAvailable blocks: "
        << pool.availableBlocks() << '\n';

    std::cout << "Allocated blocks: "
        << pool.allocatedBlocks() << '\n';


    // Demonstrate double-deallocation protection

    std::cout << "\nAttempting double deallocation...\n";

    bool firstDeallocation = pool.deallocate(packet1);
    bool secondDeallocation = pool.deallocate(packet1);

    std::cout << "First deallocation: "
        << (firstDeallocation ? "accepted" : "rejected")
        << '\n';

    std::cout << "Second deallocation: "
        << (secondDeallocation ? "accepted" : "rejected")
        << '\n';

    if (!secondDeallocation)
    {
        std::cout << "Double deallocation rejected.\n";
    }


    // Clean up remaining allocated blocks

    pool.deallocate(packet3);
    pool.deallocate(packet4);

    for (void* block : extraBlocks)
    {
        pool.deallocate(block);
    }

    std::cout << "\nFinal available blocks: "
        << pool.availableBlocks() << '\n';

    std::cout << "Final allocated blocks: "
        << pool.allocatedBlocks() << '\n';

    return 0;
}