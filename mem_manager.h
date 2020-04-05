#include <cstddef>
#include <iostream>
#include <array>
#include <cstdint>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <bitset>

using word_t = intptr_t;


struct Block
{
    size_t size;
    bool used;
    Block *next;
    Block* nextfree;
    word_t data[1];
};

struct Tuple
{
    Block *block_size;
    Block *block_remaining;
};

#define BLOCK_SIZE sizeof(Block)
#define DATA_SIZE sizeof(std::declval<Block>().data)

class MemoryManager
{
public:
    Block *splitAllocate(Block *, size_t);
    word_t *alloc(size_t);
    size_t align(size_t);
    static Block *heapStart;
    static Block *freeStart;
    static Block *top; // = heapStart;
    static size_t total_requested_memory;
    size_t getHeapSize();
    size_t getfreeSize();
    void printHeap();
    size_t allocSize(size_t size);
    Block *requestFromOS(size_t size);
    bool canCoalesce(Block *);
    Block *coalesce(Block *block);
    Tuple split(Block *block, size_t size);
    bool canSplit(Block *block, size_t size);
    Block *firstFit(size_t size);
    Block *bestFit(size_t size);
    Block *findBlock(size_t size);
    Block *getHeader(word_t *data);
    void free(word_t *data);
    void fl_add(Block *b);
    int fl_remove(Block *b);
    void printFreeList();

    bool setUsed(Block*, bool);
    bool getUsed(Block*);

    static int total_fl_add_calls;
    static int total_fl_remove_calls;
    static double total_fl_operations_time;

    MemoryManager();
    virtual ~MemoryManager();
    
};