#include <unistd.h>
#include <cstring>
#include <sys/mman.h>



#define INITIAL_BLOCK_SIZE 128 * 1024
#define INITIAL_BLOCKS 32
#define BASE_BLOCK_SIZE 128
#define MIN_ORDER 0
#define MAX_ORDER 10
#define BASE 2
#define MAX_SIZE 100000000
#define SIZE_CHECK_LIMIT(size) (size > 0 && size <= MAX_SIZE)

int powerOfBase(int power)
{
    int res = 1;
    for (int i = 0; i < power; i++)
    {
        res *= BASE;
    }
    return res;
}

typedef struct MallocMetadata
{
    size_t size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
} MallocMetadata;

class MallocManager
{
private:
    MallocManager() : head_map(nullptr), tail_map(nullptr), _num_free_blocks(0), _num_free_bytes(0),
                      _num_allocated_blocks(0), _num_allocated_bytes(0), _num_meta_data_bytes(0)
    {
        for (int i = 0; i < MAX_ORDER + 1; i++)
        {
            free_list[i] = nullptr;
        }
        //a hack to make sure the first block is aligned to 128 bytes (im scared of the alignment)
        unsigned long p = (unsigned long)sbrk(0);
        unsigned long extra_room = INITIAL_BLOCK_SIZE * INITIAL_BLOCKS - p % (INITIAL_BLOCK_SIZE * INITIAL_BLOCKS);
        free_list[MAX_ORDER] = (MallocMetadata *)sbrk(INITIAL_BLOCK_SIZE * INITIAL_BLOCKS + extra_room);
        // free_list[MAX_ORDER] += (char *)extra_room; 
        // shift free_list[MAX_ORDER] to the right by extra_room
        free_list[MAX_ORDER] = (MallocMetadata *)((char *)free_list[MAX_ORDER] + extra_room);
        MallocMetadata *new_block = free_list[MAX_ORDER];
        for (int i = 0; i < INITIAL_BLOCKS; i++)
        {
            new_block->size = INITIAL_BLOCK_SIZE;
            new_block->is_free = true;
            new_block->prev = (i == 0) ? nullptr : new_block - 1;
            new_block->next = (i == INITIAL_BLOCKS - 1) ? nullptr : new_block + 1;
            new_block = new_block->next;
        }
        _num_free_blocks = INITIAL_BLOCKS;
        _num_free_bytes = INITIAL_BLOCK_SIZE * INITIAL_BLOCKS;
        _num_allocated_blocks = INITIAL_BLOCKS;
        _num_allocated_bytes = INITIAL_BLOCK_SIZE * INITIAL_BLOCKS;
        _num_meta_data_bytes = INITIAL_BLOCKS * _size_meta_data();
    }
    static MallocMetadata &instance;

public:
    MallocMetadata *head_map;
    MallocMetadata *tail_map;

    MallocMetadata *free_list[MAX_ORDER + 1];



    size_t _num_free_blocks;
    size_t _num_free_bytes;
    size_t _num_allocated_blocks;
    size_t _num_allocated_bytes;
    size_t _num_meta_data_bytes;

    MallocManager(MallocManager const &) = delete;
    void operator=(MallocManager const &) = delete;
    static MallocManager &getInstance()
    {
        static MallocManager instance;
        return instance;
    }
    ~MallocManager() = default;
};

void *smalloc(size_t size)
{
    if (!SIZE_CHECK_LIMIT(size))
    {
        return NULL;
    }
    MallocManager &manager = MallocManager::getInstance();
    int order = getOrder(size);
    if (order > MAX_ORDER) // size is too big so need mmap()
    {
        MallocMetadata *block = mmap(nullptr, size + _size_meta_data(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        MallocMetadata temp = {size, false, manager.head_map, nullptr};
        *block = temp;
        manager.head_map = block;
        return (void *)((char *)block + _size_meta_data());

    }
    // return a block from the free list
    MallocMetadata *block = getBlockByOrder(manager.free_list, order);
    if (block == nullptr)
    {
        return nullptr;
    }
    block->is_free = false;
    manager._num_free_blocks--;
    manager._num_free_bytes -= block->size;
    return (void *)((char *)block + _size_meta_data());
}

int getOrder(size_t size)
{
    int order = MIN_ORDER;
    while (size + _size_meta_data()> BASE_BLOCK_SIZE * powerOfBase(order))
    {
        order++;
    }
    return order;
}

/*If we reuse freed memory sectors with bigger sizes than required, we’ll be wasting memory
(internal fragmentation).
Solution: Implement a function that smalloc() will use, such that if a pre-allocated block
is reused and is large enough, the function will cut the block in half to two blocks (buddies)
with two separate meta-data structs. One will serve the current allocation, and the other will
remain unused for later (marked free and added to the free blocks data structure). This
process should be done iteratively until the allocated block is no longer “large enough”.
Definition of “large enough”: The allocated block is large enough if it is of order > 0, and if
after splitting it to two equal sized blocks, the requested user allocation is small enough to
fit entirely inside the first block, so the second block will be free.
*/
MallocMetadata *getBlockByOrder(MallocMetadata **blocks_list, int order)
{
    if (blocks_list[order] != nullptr)
    {
        MallocMetadata *temp = blocks_list[order];
        blocks_list[order] = blocks_list[order]->next;
        temp->is_free = false;
        return temp;
    }
    if (order == MAX_ORDER)
    {
        return nullptr;
    }
    MallocMetadata *bigger_block = getBlockByOrder(blocks_list, order + 1);
    if (bigger_block == nullptr)
    {
        return nullptr;
    }
    // split the block into 2 insert them to the small order list delete the bigger block from the bigger order list and return the first block
    MallocMetadata *first_block = bigger_block;
    MallocMetadata *second_block = (MallocMetadata *)((char *)bigger_block + (INITIAL_BLOCK_SIZE * powerOfBase(order)));
    first_block->size = INITIAL_BLOCK_SIZE * powerOfBase(order);
    second_block->size = INITIAL_BLOCK_SIZE * powerOfBase(order);
    first_block->is_free = true;
    second_block->is_free = true;
    first_block->next = second_block;
    second_block->next = nullptr;
    first_block->prev = nullptr;
    second_block->prev = first_block;
    blocks_list[order] = first_block;
    return first_block;
}
// void mergeBlocks() {}

/*void *smalloc(size_t size)
{
    if (!SIZE_CHECK_LIMIT(size))
    {
        return NULL;
    }
    MallocMetadata *temp = MallocManager::getInstance().head;
    while (temp != nullptr)
    {
        if (size <= temp->size && temp->is_free)
        {
            temp->is_free = false;
            MallocManager &manager = MallocManager::getInstance();
            manager._num_free_blocks--;
            manager._num_free_bytes -= temp->size;
            return (void *)((char *)temp + _size_meta_data());
        }
        temp = temp->next;
    }
    // if we got here we need to allocate new memory
    MallocMetadata *new_block = (MallocMetadata *)sbrk(size + _size_meta_data());
    if (new_block == (void *)-1)
    {
        return NULL;
    }
    new_block->size = size;
    new_block->is_free = false;
    new_block->next = nullptr;
    new_block->prev = MallocManager::getInstance().tail;
    if (MallocManager::getInstance().head == nullptr)
    {
        MallocManager::getInstance().head = new_block;
    }
    if (MallocManager::getInstance().tail != nullptr)
    {
        MallocManager::getInstance().tail->next = new_block;
    }
    MallocManager::getInstance().tail = new_block;
    MallocManager::getInstance()._num_allocated_blocks++;
    MallocManager::getInstance()._num_allocated_bytes += size;
    MallocManager::getInstance()._num_meta_data_bytes += _size_meta_data();
    return (void *)((char *)new_block + _size_meta_data());
}*/

void *scalloc(size_t num, size_t size)
{
    void *ptr = smalloc(num * size);
    if (ptr == NULL)
    {
        return NULL;
    }
    memset(ptr, 0, num * size);
    return ptr;
}

void sfree(void *p)
{
    MallocMetadata* block = (MallocMetadata *)((char *)p - _size_meta_data());
    //delete the block from the heap and from the list inside manager
    if (block->size > INITIAL_BLOCK_SIZE)
    {
        
    }
    //check if nightbor is free with xor and merge
    MallocMetadata *buddy = (MallocMetadata *)((unsigned long)block ^ ((MallocMetadata *)block)->size);
    if (buddy->is_free)
    {
        //merge
        //if buddy is head
        removeBlockFromFreeList(buddy);
        //remove buddy from free list
        block->size *= BASE;
    }
    addBlockToFreeList(block);
}

// join 2 blocks to one block and add them to the free list and use addBlockToFreeList recursively
MallocMetadata* addBlockToFreeList(MallocMetadata *block)
{
    MallocManager &manager = MallocManager::getInstance();
    int order = getOrder(block->size);
    if (order == MAX_ORDER)
    {
        return block;
    }
    MallocMetadata *buddy = (MallocMetadata *)((unsigned long)block ^ ((MallocMetadata *)block)->size);
    if (buddy->is_free)
    {
        //merge
        //if buddy is head
        removeBlockFromFreeList(buddy);
        //remove buddy from free list
        block->size *= BASE;
        return addBlockToFreeList(block);
    }
    else
    {
        //add block to free list
        if (manager.free_list[order] == nullptr)
        {
            manager.free_list[order] = block;
            block->next = nullptr;
            block->prev = nullptr;
        }
        else
        {
            block->next = manager.free_list[order];
            block->prev = nullptr;
            manager.free_list[order]->prev = block;
            manager.free_list[order] = block;
        }
        manager._num_free_blocks++;
        manager._num_free_bytes += block->size;
        return block;
    }
}

void removeBlockFromFreeList(MallocMetadata *block)
{
    MallocManager &manager = MallocManager::getInstance();
    int order = getOrder(block->size);
    if (block->prev == nullptr)
    {
        manager.free_list[order] = block->next;
    }
    else
    {
        block->prev->next = block->next;
    }
    if (block->next != nullptr)
    {
        block->next->prev = block->prev;
    }
    manager._num_free_blocks--;
    manager._num_free_bytes -= block->size;
}

/**
 * @brief 
 * void sfree(void *p)
{
    if (p == NULL)
    {
        return;
    }
    MallocMetadata *temp = (MallocMetadata *)((char *)p - _size_meta_data());
    if (temp->is_free)
    {
        return;
    }
    temp->is_free = true;
    MallocManager &manager = MallocManager::getInstance();
    manager._num_free_blocks++;
    manager._num_free_bytes += temp->size;
    return;
}
 * 
 * @param oldp 
 * @param size 
 * @return void* 
 */

void *srealloc(void *oldp, size_t size)
{
    if (oldp == NULL)
    {
        return smalloc(size);
    }
    MallocMetadata *old_block = (MallocMetadata *)((char *)oldp - _size_meta_data());
    if (old_block->size >= size + _size_meta_data())
    {
        return oldp;
    }
    // is there buddies to merge?
    // if yes merge
    // int iterations = (size + _size_meta_data()) / old_block->size - ((size + _size_meta_data())%old_block->size == 0);
    if (buddiesMergeCounter(old_block, size) + getOrder(old_block->size) > getOrder(size))
    {
        mergeBudies(old_block, getOrder(size) - getOrder(old_block->size));
    }
    else
    {
        //smalloc new block
        smalloc(size);
        //copy data
        memset(oldp, 0, old_block->size);
        //sfree old block
        sfree(oldp);
    }
    
}

void mergeBudies(MallocMetadata *old_block, int num_of_iterations){
    MallocMetadata *buddy = (MallocMetadata *)((unsigned long)old_block ^ ((MallocMetadata *)old_block)->size);
    for (int i = 0; i < num_of_iterations; i++)
    {
        removeBlockFromFreeList(buddy);
        old_block->size *= BASE;
        buddy = (MallocMetadata *)((unsigned long)old_block ^ ((MallocMetadata *)old_block)->size);
    }
    addBlockToFreeList(old_block);
}


int buddiesMergeCounter(MallocMetadata * old_block, size_t size){
    int counter = 0;
    int order = getOrder(size);
    MallocMetadata *buddy = (MallocMetadata *)((unsigned long)old_block ^ ((MallocMetadata *)old_block)->size);
    while (buddy->is_free && order < MAX_ORDER)
    {
        counter++;
        order++;
        buddy = (MallocMetadata *)((unsigned long)buddy ^ ((MallocMetadata *)buddy)->size * BASE);
    }
    return counter;
}


// void *srealloc(void *oldp, size_t size)
// {
//     if (oldp == NULL)
//     {
//         return smalloc(size);
//     }
//     MallocMetadata *temp = (MallocMetadata *)((char *)oldp - _size_meta_data());
//     if (temp->size >= size)
//     {
//         return oldp;
//     }
//     void *newp = smalloc(size);
//     if (newp == NULL)
//     {
//         return NULL;
//     }
//     memcpy(newp, oldp, temp->size);
//     sfree(oldp);
//     return newp;
// }

size_t _num_free_blocks()
{
    return MallocManager::getInstance()._num_free_blocks;
}

size_t _num_free_bytes()
{
    return MallocManager::getInstance()._num_free_bytes;
}

size_t _num_allocated_blocks()
{
    return MallocManager::getInstance()._num_allocated_blocks;
}

size_t _num_allocated_bytes()
{
    return MallocManager::getInstance()._num_allocated_bytes;
}

size_t _num_meta_data_bytes()
{
    return MallocManager::getInstance()._num_meta_data_bytes;
}

size_t _size_meta_data()
{
    return sizeof(MallocMetadata);
}
