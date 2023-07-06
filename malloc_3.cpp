#include <unistd.h>
#include <cstring>
#include <sys/mman.h>
#include <cstdlib>
#include <iostream>

#define INITIAL_BLOCK_SIZE 128 * 1024
#define INITIAL_BLOCKS 32
#define BASE_BLOCK_SIZE 128
#define MIN_ORDER 0
#define MAX_ORDER 10
#define BASE 2
#define MAX_SIZE 1e8
#define SIZE_CHECK_LIMIT(size) (size > 0 && size <= MAX_SIZE)

size_t _size_meta_data();

typedef struct MallocMetadata
{
    int cookies;
    size_t size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
} MallocMetadata;

class MallocManager
{
private:
    MallocManager() : major_cookie(0), head_map(nullptr), _num_free_blocks(INITIAL_BLOCKS),
                      _num_free_bytes(INITIAL_BLOCKS * INITIAL_BLOCK_SIZE), _num_allocated_blocks(INITIAL_BLOCKS),
                      _num_allocated_bytes(INITIAL_BLOCKS * INITIAL_BLOCK_SIZE), _num_meta_data_bytes(0)
    {
        major_cookie = rand();
        for (int i = 0; i < MAX_ORDER + 1; i++)
        {
            free_list[i] = nullptr;
        }
        // a hack to make sure the first block is aligned to 128 bytes (im scared of the alignment)
        unsigned long p = (unsigned long)sbrk(0);
        unsigned long extra_room = INITIAL_BLOCK_SIZE * INITIAL_BLOCKS - p % (INITIAL_BLOCK_SIZE * INITIAL_BLOCKS);
        free_list[MAX_ORDER] = (MallocMetadata *)sbrk(INITIAL_BLOCK_SIZE * INITIAL_BLOCKS + extra_room);
        // free_list[MAX_ORDER] += (char *)extra_room;
        // shift free_list[MAX_ORDER] to the right by extra_room
        free_list[MAX_ORDER] = (MallocMetadata *)((char *)free_list[MAX_ORDER] + extra_room);
        MallocMetadata *new_block = free_list[MAX_ORDER];
        for (int i = 0; i < INITIAL_BLOCKS; i++)
        {
            new_block->cookies = major_cookie;
            new_block->size = INITIAL_BLOCK_SIZE;
            new_block->is_free = true;
            new_block->prev = (i == 0) ? nullptr : (MallocMetadata *)((char *)new_block - INITIAL_BLOCK_SIZE);
            // new_block->prev = (i == 0) ? nullptr : new_block - 1;
            new_block->next = (i == INITIAL_BLOCKS - 1) ? nullptr : (MallocMetadata *)((char *)new_block + INITIAL_BLOCK_SIZE);
            // new_block->next = (i == INITIAL_BLOCKS - 1) ? nullptr : new_block + 1;
            new_block = new_block->next;
        }
        // _num_free_blocks = INITIAL_BLOCKS;
        // _num_free_bytes = INITIAL_BLOCK_SIZE * INITIAL_BLOCKS;
        // _num_allocated_blocks = INITIAL_BLOCKS;
        // _num_allocated_bytes = INITIAL_BLOCK_SIZE * INITIAL_BLOCKS;
        _num_meta_data_bytes = INITIAL_BLOCKS * _size_meta_data();
        // std::cout << "First c'tor: num free blocks:" << _num_free_blocks << std::endl;
    }

public:
    int major_cookie;
    MallocMetadata *head_map;

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
        // //std::cout << "get instance" << std::endl;
        static MallocManager instance;
        // //std::cout << "after get instance" << std::endl;
        return instance;
    }
    ~MallocManager() = default;
};

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
    return MallocManager::getInstance()._num_allocated_blocks * _size_meta_data();
    // return MallocManager::getInstance()._num_meta_data_bytes;
}

size_t _size_meta_data()
{
    return sizeof(MallocMetadata);
}

int powerOfBase(int power)
{
    int res = 1;
    for (int i = 0; i < power; i++)
    {
        res *= BASE;
    }
    return res;
}

void tasteCookie(MallocMetadata *block)
{
    // //std::cout << "taste cookie" << std::endl;
    if (block->cookies != MallocManager::getInstance().major_cookie)
    {
        // //std::cout << "cookie: " << block->cookies << std::endl;
        exit(0xdeadbeef);
    }
    // //std::cout << "after taste cookie" << std::endl;
}

int getOrder(size_t size)
{
    // //std::cout << "get order: " << size << std::endl;
    int order = MIN_ORDER;
    while (size > BASE_BLOCK_SIZE * powerOfBase(order))
    {
        // //std::cout << "get order: " << size << std::endl;
        // //std::cout << "get order: " << BASE_BLOCK_SIZE * powerOfBase(order) << std::endl;

        order++;
    }
    // //std::cout << "get order: " << order << std::endl;
    return order;
}

void removeBlockFromFreeList(MallocMetadata *block)
{
    tasteCookie(block);
    MallocManager &manager = MallocManager::getInstance();
    // std::cout << "removeBlockFromFreeList: num free blocks:" << manager._num_free_blocks << std::endl;
    // std::cout << "before changing:" << manager._num_free_blocks << std::endl;
    int order = getOrder(block->size);
    if (block->prev == nullptr) // block is head
    {
        manager.free_list[order] = block->next;
    }
    else // block is not head
    {
        block->prev->next = block->next;
    }
    if (block->next != nullptr) // block is not tail
    {
        block->next->prev = block->prev;
    }
    // std::cout << "after changing:" << manager._num_free_blocks << std::endl;
    manager._num_free_blocks--;
    block->next = block->prev = nullptr;
    block->is_free = false;
    // std::cout << "removeBlockFromFreeList: num free blocks:" << manager._num_free_blocks << std::endl;
}

MallocMetadata *addBlockToFreeList(MallocMetadata *block)
{
    tasteCookie(block);
    MallocManager &manager = MallocManager::getInstance();
    // std::cout << "addBlockToFreeList: num free blocks:" << manager._num_free_blocks << std::endl;
    int order = getOrder(block->size);
    if (order == MAX_ORDER)
    {
        manager._num_free_blocks++;
        // std::cout << "addBlockToFreeList MAX_ORDER: num free blocks:" << manager._num_free_blocks << std::endl;
        if (manager.free_list[order] == nullptr)
        {
            manager.free_list[order] = block;
            block->next = nullptr;
            block->prev = nullptr;
        }
        else
        {
            tasteCookie(manager.free_list[order]);
            block->next = manager.free_list[order];
            block->next->prev = block;
            block->prev = nullptr;
            manager.free_list[order] = block;
        }
        return block;
    }
    MallocMetadata *buddy = (MallocMetadata *)((unsigned long)block ^ ((MallocMetadata *)block)->size);
    tasteCookie(buddy);
    if (buddy->is_free && getOrder(buddy->size) == order) // buddy is free and same order
    {
        removeBlockFromFreeList(buddy);
        block->size *= BASE;
        manager._num_allocated_blocks--;
        manager._num_meta_data_bytes -= _size_meta_data();
        return addBlockToFreeList((MallocMetadata *)block);
    }
    else
    {
        manager._num_free_blocks++;
        if (manager.free_list[order] == nullptr)
        {
            manager.free_list[order] = block;
            block->next = nullptr;
            block->prev = nullptr;
        }
        else
        {
            tasteCookie(manager.free_list[order]);
            block->next = manager.free_list[order];
            block->next->prev = block;
            block->prev = nullptr;
            manager.free_list[order] = block;
        }
        return block;
    }
}

MallocMetadata *getBlockByOrder(MallocMetadata **blocks_list, int order)
{
    MallocManager &manager = MallocManager::getInstance();
    // std::cout << "getBlockByOrder: num free blocks:" << manager._num_free_blocks << std::endl;
    if (blocks_list[order] != nullptr)
    {
        MallocMetadata *temp = blocks_list[order];
        removeBlockFromFreeList(temp);
        return temp;
    }
    if (order == MAX_ORDER) // no bigger blocks to split
    {
        return nullptr;
    }
    MallocMetadata *bigger_block = getBlockByOrder(blocks_list, order + 1);
    if (bigger_block == nullptr)
    {
        return nullptr;
    }
    MallocMetadata *first_block = bigger_block;
    MallocMetadata *second_block = (MallocMetadata *)((char *)bigger_block + bigger_block->size / BASE);
    second_block->cookies = first_block->cookies;
    first_block->size = second_block->size = bigger_block->size / BASE;
    first_block->is_free = false;
    second_block->is_free = true;
    second_block->prev = second_block->next = first_block->prev = first_block->next = nullptr;
    addBlockToFreeList(second_block);
    manager._num_allocated_blocks++;
    manager._num_meta_data_bytes += _size_meta_data();
    return first_block;
}

void mergeBudies(MallocMetadata *old_block, int num_of_iterations)
{
    // std::cout << "mergeBudies: num free blocks:" << MallocManager::getInstance()._num_free_blocks << std::endl;
    tasteCookie(old_block);
    MallocMetadata *buddy = (MallocMetadata *)((unsigned long)old_block ^ ((MallocMetadata *)old_block)->size);
    for (int i = 0; i < num_of_iterations; i++)
    {
        tasteCookie(buddy);
        removeBlockFromFreeList(buddy);
        MallocManager::getInstance()._num_meta_data_bytes -= _size_meta_data();
        old_block->size *= BASE;
        buddy = (MallocMetadata *)((unsigned long)old_block ^ ((MallocMetadata *)old_block)->size);
    }
    addBlockToFreeList(old_block);
}

int buddiesMergeCounter(MallocMetadata *old_block, size_t size)
{
    tasteCookie(old_block);
    int counter = 0;
    int order = getOrder(size);
    MallocMetadata *buddy = (MallocMetadata *)((unsigned long)old_block ^ ((MallocMetadata *)old_block)->size);
    while (buddy->is_free && order < MAX_ORDER)
    {
        tasteCookie(buddy);
        counter++;
        order++;
        buddy = (MallocMetadata *)((unsigned long)buddy ^ ((MallocMetadata *)buddy)->size * BASE);
    }
    return counter;
}

void *smalloc(size_t size)
{
    if (!SIZE_CHECK_LIMIT(size))
    {
        return NULL;
    }
    MallocManager &manager = MallocManager::getInstance();
    // std::cout << "smalloc: num free blocks:" << manager._num_free_blocks << std::endl;
    int order = getOrder(size + _size_meta_data());
    if (order > MAX_ORDER) // size is too big so need mmap()
    {
        MallocMetadata *block = (MallocMetadata *)mmap(nullptr, size + _size_meta_data(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        MallocMetadata temp = {MallocManager::getInstance().major_cookie,
                               (unsigned long)size + _size_meta_data(), false,
                               nullptr, nullptr};
        *block = temp;
        block->next = manager.head_map;
        manager.head_map = block;
        manager._num_allocated_blocks++;
        manager._num_allocated_bytes += size + _size_meta_data();
        manager._num_meta_data_bytes += _size_meta_data();
        block->is_free = false;
        return (void *)(block + 1);
    }
    MallocMetadata *block = getBlockByOrder(manager.free_list, order);
    if (block == nullptr)
    {
        return nullptr;
    }
    tasteCookie(block);
    block->is_free = false;
    // manager._num_free_blocks--;
    manager._num_free_bytes -= block->size;
    // std::cout << "smalloc: num free blocks:" << manager._num_free_blocks << std::endl;
    return (void *)(block + 1);
}

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
    if (!p)
    {
        return;
    }
    MallocMetadata *block = (MallocMetadata *)p - 1;
    tasteCookie(block);
    MallocManager &manager = MallocManager::getInstance();
    if (block->size > INITIAL_BLOCK_SIZE)
    {
        manager._num_allocated_bytes -= block->size;
        manager._num_allocated_blocks--;
        manager._num_meta_data_bytes -= _size_meta_data();
        // remove block from linked list
        if (block->prev == nullptr)
        {
            MallocManager::getInstance().head_map = block->next;
        }
        else
        {
            tasteCookie(block->prev);
            block->prev->next = block->next;
        }
        if (block->next != nullptr)
        {
            tasteCookie(block->next);
            block->next->prev = block->prev;
        }
        munmap(block, block->size);
        return;
    }
    MallocManager::getInstance()._num_free_bytes += block->size;
    addBlockToFreeList(block);
}

void *srealloc(void *oldp, size_t size)
{
    if (oldp == NULL)
    {
        return smalloc(size);
    }
    MallocMetadata *old_block = (MallocMetadata *)((MallocMetadata *)oldp - 1);
    tasteCookie(old_block);
    if (old_block->size >= size + _size_meta_data())
    {
        return oldp;
    }
    // is there buddies to merge?
    // if yes merge
    // int iterations = (size + _size_meta_data()) / old_block->size - ((size + _size_meta_data())%old_block->size == 0);
    if (old_block->size <= INITIAL_BLOCK_SIZE && buddiesMergeCounter(old_block, size) + getOrder(old_block->size) > getOrder(size))
    {
        mergeBudies(old_block, getOrder(size + _size_meta_data()) - getOrder(old_block->size));
        return oldp;
    }
    else
    {
        // smalloc new block
        void *newp = smalloc(size);
        // copy data
        memset(oldp, 0, old_block->size);
        // sfree old block
        sfree(oldp);
        return newp;
    }
}
