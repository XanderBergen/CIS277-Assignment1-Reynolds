# CIS-277 Assignment 1: Network Packet Buffer Pool

## Student
Alexander Reynolds

## Description
This project implements a fixed-size network packet memory pool using a custom Stack ADT. The pool reserves memory at construction, allocates and deallocates blocks in O(1) time, supports binary packet data, rejects invalid and duplicate deallocations, and uses LIFO behavior.

## Stack Implementation
Linked Structure. I chose a linked structure because it keeps the push() and pop() operations simple, only needing to either create a new node at the top, or remove the top node. A linked structure also means I don't need to worry about the size of the array, or resizing it at runtime.

## How to Compile
Compile with a C++ 17 compiler with the following command:
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp MemoryPool.cpp -o buffer_pool

## How to Run
After compiling, run the program with ./buffer_pool, or buffer_pool.exe

## Analysis Questions
1. A Stack is appropriate for this due to its last-in, first-out (LIFO) behavior. When a block of data is released, it is pushed onto the Stack, allowing the next allocation to pop that same block for reuse. 
2. When the free-block Stack becomes empty, there are no available memory blocks. In this case, allocate() will return a nullptr, indicating that there are no available blocks.
3. If released blocks were not returned to the Stack then those blocks could not be used by future allocations. Over time the pool will run out of available blocks, even if some of those blocks are no longer being used and could be filled with other data.
4. If the same block got deallocated twice, it could lead to its address being placed on the free-block Stack more than once. This could lead to a single memory block being allocated to two users at the same time, which can corrupt data, or lead to other incorrect memory behavior.
5. The time complexity of allocate() is O(1). Allocate checks whether the Stack is empty, removes the top block using pop(), which is also O(1), marks that block as being allocated, and then returns its address. As none of these operations require searching through the pool, they do not increase in time complexity as the pool grows, meaning allocate() is O(1).
6. The time complexity of deallocate() is O(1). Deallocate calculates the block's index from its address, without searching through the pool. It then checks if the block is valid and currently allocated. Then it pushes the block back onto the free-block Stack. As none of these operations require searching through the pool, they do not increase in time complexity as the pool grows, meaning deallocate() is O(1).