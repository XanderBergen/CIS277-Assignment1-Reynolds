#include <iostream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <cstddef>

#include "MemoryPool.h"

int main()
{
    const std::size_t blockSize = 512;
    const std::size_t blockCount = 8;

    MemoryPool pool(blockSize, blockCount);

    int testsPassed = 0;
    int testsFailed = 0;

    auto check = [&](bool condition, const char* testName)
        {
            if (condition)
            {
                std::cout << "[PASS] " << testName << '\n';
                ++testsPassed;
            }
            else
            {
                std::cout << "[FAIL] " << testName << '\n';
                ++testsFailed;
            }
        };

    std::cout << "Network Packet Buffer Pool\n\n";

    std::cout << "Block Size:       "
        << pool.blockSize() << " bytes\n";

    std::cout << "Blocks:           "
        << blockCount << '\n';

    std::cout << "Total Capacity:   "
        << pool.capacity() << " bytes\n\n";


    // ---------------------------------------------------------
    // Initial pool tests
    // ---------------------------------------------------------

    std::cout << "--- Initial Pool Tests ---\n";

    check(pool.blockSize() == blockSize,
        "blockSize() returns the correct value");

    check(pool.capacity() == blockSize * blockCount,
        "capacity() returns the correct value");

    check(pool.availableBlocks() == blockCount,
        "All blocks are initially available");

    check(pool.allocatedBlocks() == 0,
        "No blocks are initially allocated");


    // ---------------------------------------------------------
    // Allocate several blocks
    // ---------------------------------------------------------

    std::cout << "\n--- Allocation Tests ---\n";

    void* packet1 = pool.allocate();
    void* packet2 = pool.allocate();
    void* packet3 = pool.allocate();

    std::cout << "Packet 1 allocated: " << packet1 << '\n';
    std::cout << "Packet 2 allocated: " << packet2 << '\n';
    std::cout << "Packet 3 allocated: " << packet3 << '\n';

    check(packet1 != nullptr,
        "Packet 1 allocation succeeded");

    check(packet2 != nullptr,
        "Packet 2 allocation succeeded");

    check(packet3 != nullptr,
        "Packet 3 allocation succeeded");

    check(packet1 != packet2 &&
        packet1 != packet3 &&
        packet2 != packet3,
        "Allocated blocks have unique addresses");

    check(pool.availableBlocks() == 5,
        "Five blocks remain after three allocations");

    check(pool.allocatedBlocks() == 3,
        "Three blocks are reported as allocated");

    std::cout << "\nAvailable blocks: "
        << pool.availableBlocks() << '\n';

    std::cout << "Allocated blocks: "
        << pool.allocatedBlocks() << '\n';


    // ---------------------------------------------------------
    // Binary data test
    // ---------------------------------------------------------

    std::cout << "\n--- Binary Data Test ---\n";

    unsigned char packetData[] =
    {
        0x45, 0x00, 0x00, 0x3C,
        0xAB, 0xCD, 0x12, 0x34
    };

    bool binaryTestPassed = false;

    if (packet1 != nullptr &&
        sizeof(packetData) <= pool.blockSize())
    {
        std::memcpy(packet1,
            packetData,
            sizeof(packetData));

        std::cout << "Binary packet written to Packet 1.\n";

        unsigned char* storedData =
            static_cast<unsigned char*>(packet1);

        std::cout << "Packet 1 data: ";

        for (std::size_t i = 0;
            i < sizeof(packetData);
            ++i)
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
            << '\n';

        binaryTestPassed =
            std::memcmp(packet1,
                packetData,
                sizeof(packetData)) == 0;
    }

    check(binaryTestPassed,
        "Binary data can be written and read back correctly");


    // ---------------------------------------------------------
    // Memory reuse test
    // ---------------------------------------------------------

    std::cout << "\n--- Memory Reuse Test ---\n";

    void* releasedAddress = packet2;

    bool packet2Released =
        pool.deallocate(packet2);

    check(packet2Released,
        "Packet 2 was successfully released");

    check(pool.availableBlocks() == 6,
        "Available count increased after deallocation");

    check(pool.allocatedBlocks() == 2,
        "Allocated count decreased after deallocation");

    void* packet4 = pool.allocate();

    std::cout << "Released address:   "
        << releasedAddress << '\n';

    std::cout << "Packet 4 allocated: "
        << packet4 << '\n';

    check(packet4 == releasedAddress,
        "Packet 4 reused the most recently released block");


    // ---------------------------------------------------------
    // Invalid deallocation tests
    // ---------------------------------------------------------

    std::cout << "\n--- Invalid Deallocation Tests ---\n";

    check(!pool.deallocate(nullptr),
        "nullptr deallocation is rejected");

    int outsideVariable = 123;

    check(!pool.deallocate(&outsideVariable),
        "Pointer outside the memory pool is rejected");

    if (packet1 != nullptr)
    {
        unsigned char* middleOfBlock =
            static_cast<unsigned char*>(packet1) + 1;

        check(!pool.deallocate(middleOfBlock),
            "Pointer into the middle of a block is rejected");
    }


    // ---------------------------------------------------------
    // Exhaustion tests
    // ---------------------------------------------------------

    std::cout << "\n--- Pool Exhaustion Tests ---\n";

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

    check(pool.availableBlocks() == 0,
        "No blocks remain after pool exhaustion");

    check(pool.allocatedBlocks() == blockCount,
        "All blocks are allocated after exhaustion");

    void* failedAllocation = pool.allocate();

    check(failedAllocation == nullptr,
        "allocate() returns nullptr when pool is exhausted");

    // Test it again to make sure exhaustion does not alter state.
    void* secondFailedAllocation = pool.allocate();

    check(secondFailedAllocation == nullptr,
        "Repeated allocation on exhausted pool returns nullptr");

    check(pool.availableBlocks() == 0,
        "Failed allocations do not change available block count");

    check(pool.allocatedBlocks() == blockCount,
        "Failed allocations do not change allocated block count");


    // ---------------------------------------------------------
    // Double-deallocation test
    // ---------------------------------------------------------

    std::cout << "\n--- Double Deallocation Test ---\n";

    bool firstDeallocation =
        pool.deallocate(packet1);

    bool secondDeallocation =
        pool.deallocate(packet1);

    check(firstDeallocation,
        "First deallocation is accepted");

    check(!secondDeallocation,
        "Second deallocation of same block is rejected");

    check(pool.availableBlocks() == 1,
        "Exactly one block becomes available");

    check(pool.allocatedBlocks() == blockCount - 1,
        "Allocated count decreases exactly once");


    // ---------------------------------------------------------
    // Reallocation after double-deallocation test
    // ---------------------------------------------------------

    std::cout << "\n--- Reallocation Test ---\n";

    void* packet1Again = pool.allocate();

    std::cout << "Original Packet 1 address: "
        << packet1 << '\n';

    std::cout << "Reallocated address:       "
        << packet1Again << '\n';

    check(packet1Again == packet1,
        "Released Packet 1 block is reused");

    check(pool.availableBlocks() == 0,
        "Pool is exhausted again after reallocation");

    check(pool.allocatedBlocks() == blockCount,
        "All blocks are allocated again");


    // ---------------------------------------------------------
    // Cleanup
    // ---------------------------------------------------------

    std::cout << "\n--- Cleanup Tests ---\n";

    check(pool.deallocate(packet1Again),
        "Reallocated Packet 1 cleaned up");

    check(pool.deallocate(packet3),
        "Packet 3 cleaned up");

    check(pool.deallocate(packet4),
        "Packet 4 cleaned up");

    bool extraCleanupSucceeded = true;

    for (void* block : extraBlocks)
    {
        if (!pool.deallocate(block))
        {
            extraCleanupSucceeded = false;
        }
    }

    check(extraCleanupSucceeded,
        "All additional blocks cleaned up");

    check(pool.availableBlocks() == blockCount,
        "All blocks are available after cleanup");

    check(pool.allocatedBlocks() == 0,
        "No blocks remain allocated after cleanup");


    // ---------------------------------------------------------
    // Final results
    // ---------------------------------------------------------

    std::cout << "\n=================================\n";
    std::cout << "Test Summary\n";
    std::cout << "=================================\n";

    std::cout << "Tests passed: " << testsPassed << '\n';
    std::cout << "Tests failed: " << testsFailed << '\n';

    std::cout << "\nFinal available blocks: "
        << pool.availableBlocks() << '\n';

    std::cout << "Final allocated blocks: "
        << pool.allocatedBlocks() << '\n';

    if (testsFailed == 0)
    {
        std::cout << "\nAll tests passed successfully.\n";
    }
    else
    {
        std::cout << "\nOne or more tests failed.\n";
    }

    return testsFailed == 0 ? 0 : 1;
};