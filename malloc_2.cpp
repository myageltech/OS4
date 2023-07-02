#include <unistd.h>
#include <cstring>

#define MAX_SIZE 100000000
#define SIZE_CHECK_LIMIT(size) (size > 0 && size <= MAX_SIZE)

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
    MallocManager() : head(nullptr), tail(nullptr), _num_free_blocks(0), _num_free_bytes(0), _num_allocated_blocks(0), _num_allocated_bytes(0), _num_meta_data_bytes(0) {}
    static MallocMetadata &instance;

public:
    MallocMetadata *head;
    MallocMetadata *tail;

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

void *srealloc(void *oldp, size_t size)
{
    if (oldp == NULL)
    {
        return smalloc(size);
    }
    MallocMetadata *temp = (MallocMetadata *)((char *)oldp - _size_meta_data());
    if (temp->size >= size)
    {
        return oldp;
    }
    void *newp = smalloc(size);
    if (newp == NULL)
    {
        return NULL;
    }
    memcpy(newp, oldp, temp->size);
    sfree(oldp);
    return newp;
}

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
