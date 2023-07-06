#include <unistd.h>
#define MAX_SIZE 100000000
#define SIZE_LIMIT_CHECK(size) ((size > 0) && (size < MAX_SIZE))

void *smalloc(size_t size)
{
    if (!SIZE_LIMIT_CHECK(size))
    {
        return NULL;
    }
    void *p = sbrk(0);
    void *request = sbrk(size);
    if (request == (void *)-1)
    {
        return NULL;
    }
    else
    {
        return p;
    }
}
