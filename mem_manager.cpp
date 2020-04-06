#include <cstddef>
#include <iostream>
#include <array>
#include <cstdint>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <bitset>


#ifndef POLICY
#define POLICY 0
#endif

#ifndef SPLITIING
#define SPLITTING 0
#endif

#ifndef COALESCING
#define COALESCING 0
#endif

using namespace std;

int policy = POLICY;

/**
 * Machine word size. Depending on the architecture,
 * can be 4 or 8 bytes.
 */
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

    MemoryManager()
    {
        cout << "calling mem manager constructor" << endl;
    }
    virtual ~MemoryManager();
    
};
/*
 * Aligns the size by the machine word.
 */
inline size_t MemoryManager::align(size_t n)
{
    return (n + sizeof(word_t) - 1) & ~(sizeof(word_t) - 1);
}

/* heap start. Initialized on first allocation */
Block *MemoryManager::heapStart = nullptr;
Block *MemoryManager::freeStart = nullptr;
/* current top, updated on each allocation */
Block *MemoryManager::top = heapStart;
size_t MemoryManager::total_requested_memory = 0;
int MemoryManager::total_fl_add_calls = 0;
int MemoryManager::total_fl_remove_calls = 0;
double MemoryManager::total_fl_operations_time = 0;


bool MemoryManager::setUsed(Block* block, bool value){
    if (value){
        cout << "tried to set to true" << endl;
        block = (Block*)((uintptr_t)(void*)block | 0x03ULL);
        return true;
    }
    else{
        cout << "tried to set to false" << endl;
        block = (Block*)((uintptr_t)(void*)block & ~0x03ULL);
        return false;
    }
    
}


bool MemoryManager::getUsed(Block* block){
    auto value = 0x03ULL & (uintptr_t)(void*)block;
    if (value) cout << "yes" << endl;
    else cout << "no" << endl;
    return value;
}

size_t MemoryManager::getHeapSize()
{
    int i = 0;
    auto block = heapStart;
    while (block != nullptr)
    {
        block = block->next;
        i++;
    }
    return (size_t)i;
}

size_t MemoryManager::getfreeSize(){
    int i = 0;
    auto block = freeStart;
    while (block != nullptr)
    {
        block = block->nextfree;
        i++;
    }
    return (size_t)i;
}

void MemoryManager::printFreeList()
{
    auto block = freeStart;
    while (block != nullptr)
    {
        cout << block << ": " << block->size << "," << block->used << " ";
        block = block->nextfree;
    }
    cout << endl;
    cout << "freeSize " << getfreeSize() << endl;
}

MemoryManager::~MemoryManager()
{
        cout << "calling mem manager destructor" << endl;
        cout << "heapsize - total_remove - total_add " << getHeapSize() <<
        "-" << total_fl_remove_calls << "-" << total_fl_add_calls << endl;
        if (sbrk(-total_requested_memory) == (void *)-1)
        {
            cout << "error occured during destructor operation" << endl;
        }
}

void MemoryManager::printHeap()
{
    auto block = heapStart;
    while (block != nullptr)
    {
        cout << block << " : " << block->size << "," << block->used << " ";
        block = block->next;
    }
    cout << endl;
    cout << "heapSize " << getHeapSize() << " total_req_mem " << total_requested_memory << endl;
}


int MemoryManager::fl_remove(Block* block)
{
    total_fl_remove_calls += 1;
	if (freeStart == nullptr){
        cout << "free list is null" << endl;
        return 1;
    }
    if (freeStart == block){
        Block* tmp = freeStart->nextfree;
        freeStart->nextfree = nullptr;
        freeStart = tmp;
        return 0;
    }
    else {
        Block* prev = freeStart;
        Block* f = freeStart->nextfree;
        while (prev->nextfree != block && f!= nullptr){
            prev = prev->nextfree;
            f = f->nextfree;
        }
        if (nullptr == f){
            return 1;
        }
        else {
            prev->nextfree = f->nextfree;
            f->nextfree = nullptr;
        }
    }
    return 0;
}

void MemoryManager::fl_add(Block* block){
    total_fl_add_calls += 1;
    if(!freeStart || (unsigned long)freeStart > (unsigned long)block){    
        block->nextfree = freeStart;
        freeStart = block; 
    }
    else{
        Block* f = freeStart;
        Block* pos = freeStart;
        while (f->nextfree != nullptr && (unsigned long)f->nextfree < (unsigned long)block){
            pos = f->nextfree;
            f = f->nextfree;
        }
        block->nextfree = pos->nextfree;
        pos->nextfree = block;
    }
}

/* 
* subtract data_size because it is already counted 
* as part of block header and contributes to size
*/
inline size_t MemoryManager::allocSize(size_t size)
{
    return size + BLOCK_SIZE - DATA_SIZE;
}

/**
 * Requests (maps) memory from OS.
 * Only called when no free block exists in our
 * linked list of blocks.
 */
Block *MemoryManager::requestFromOS(size_t size)
{
    // Current heap break.
    auto block = (Block *)sbrk(0); // (1)
    // Out of Memory.
    if (sbrk(allocSize(size)) == (void *)-1)
    { // (2)
        cout << "Out of Memory!" << endl;
        return nullptr;
    }
    total_requested_memory += size;
    return block;
}

bool MemoryManager::canCoalesce(Block *block)
{
    bool retval = (block->next && !block->next->used);

    return retval;
}

/**
 * Coalesces two adjacent blocks.
 */
Block *MemoryManager::coalesce(Block *block)
{
    fl_remove(block->next);
    size_t new_size = block->size + block->next->size + BLOCK_SIZE - DATA_SIZE;
    block->next = block->next->next;
    block->size = new_size;
    return block;
}

/**
 * Splits the block in two, returns the pointer to the 
 * sub-block of size = size.
 */
Tuple MemoryManager::split(Block *block, size_t size)
{
    size_t new_size = align(size);
    size_t remaining_size = block->size - new_size - BLOCK_SIZE + DATA_SIZE;

    if (align(remaining_size) + BLOCK_SIZE - DATA_SIZE + new_size == block->size)
    {
        remaining_size = align(remaining_size);
    }
    else
    { 
        remaining_size = align(remaining_size) - sizeof(word_t);
    }
    Block *newblock = (Block*)((unsigned long)block + (unsigned long)new_size + (unsigned long)BLOCK_SIZE - (unsigned long)DATA_SIZE);
    // fl_remove(block); this is done in splitAllocate
    newblock->size = remaining_size;
    newblock->next = block->next;
    newblock->used = false;
    block->next = newblock;
    block->size = new_size;
    newblock->nextfree = nullptr;
    fl_remove(block);F
    block->used = true;
    fl_add(newblock);
    return {block, newblock};
}

/**
 * Whether this block can be split.
 */
inline bool MemoryManager::canSplit(Block *block, size_t size)
{
    bool can = false;
    size_t new_size = align(size);
    int remaining_size = (int)block->size - (int)new_size - (int)BLOCK_SIZE + (int)DATA_SIZE;
    if (remaining_size <= 0)
        return false;
    if (align(remaining_size) + BLOCK_SIZE - DATA_SIZE + new_size == block->size)
    {
        can = true;
    }
    else if ((size_t)(align(remaining_size) - sizeof(word_t)) >= sizeof(word_t))
    {
        can = true;
    }
    else
    {
        can = false;
    }
    return can;
}

Block *MemoryManager::splitAllocate(Block *block, size_t size)
{
    if (canSplit(block, size))
    {
        auto tuple = split(block, size);
        block = tuple.block_size;
    }
    if(!block->used){
        fl_remove(block);
        block->used = true;
    }
    
    return {block};
}

Block *MemoryManager::firstFit(size_t size)
{
    Block* block = freeStart;
    
    while (block != nullptr)
    {
        if (block->used || block->size < size)
        {
            block = block->nextfree;
            continue;
        }

        // Found the block:
        return block;
        // fl_remove(block); splitAllocate will do that
    }
    return nullptr;
}

Block *MemoryManager::bestFit(size_t size)
{
    Block *block = freeStart;
    size_t lowest = SIZE_MAX;
    int i = -1;
    int best_i = -1;
    while (block != nullptr)
    {
        i++;
        // O(n) search.
        if (block->used || block->size < size)
        {
            block = block->nextfree;
            continue;
        }
        else
        {
            if (block->size < lowest)
            {
                lowest = block->size;
                best_i = i;
                block = block->nextfree;
                if (lowest == size)
                    // exact fit
                    break;
            }
            continue;
        }
    }
    if (best_i >= 0)
    {
        block = freeStart;
        for (int j = 0; j < best_i; j++)
        {
            block = block->nextfree;
        }
        return block;
    }

    return nullptr;
}

Block *MemoryManager::findBlock(size_t size)
{
    switch (policy)
    {
    case 0:
    {
        auto block = firstFit(size);
        if (block)
            return splitAllocate(block, size);
        else
            return nullptr;
        break;
    }
    case 1:
    {
        auto block = bestFit(size);
        if (block)
            return splitAllocate(block, size);
        else
            return nullptr;
        break;
    }
    default:
        return firstFit(size);
        break;
    }
}

// return starting mem address of data
word_t *MemoryManager::alloc(size_t size)
{
    size = align(size);

    /* 
  * now search for an available block with
  * the appropriate size in the free 
  * blocks list (heap)
  */
    if (auto block = findBlock(size))
    {
        block->used = 1;
        return block->data;
    }
    auto block = requestFromOS(size);
    block->size = size;
    block->used = true;
    // initialize the heap if uninitialized
    if (heapStart == nullptr)
    {
        heapStart = block;
    }
    // chain the blocks together
    if (top != nullptr)
    {
        top->next = block;
    }

    top = block;

    // user payload
    return block->data;
}

/* get block metadata fron its data start address*/
Block *MemoryManager::getHeader(word_t *data)
{
    return (Block *)((char *)data + DATA_SIZE - BLOCK_SIZE);
}

void MemoryManager::free(word_t *data)
{
    auto block = getHeader(data);
    if (canCoalesce(getHeader(data)))
    {
        block = coalesce(block);
    }
    fl_add(block);
    block->used = false;
}

static MemoryManager gmm;

void* operator new(size_t size){
    return gmm.alloc(size);
}

void* operator new[] (size_t size){
    return gmm.alloc(size);
}

void operator delete(void * p){
    return gmm.free((word_t*)p);
}

void operator delete[] (void *arr){
    gmm.free((word_t*)arr);
}
