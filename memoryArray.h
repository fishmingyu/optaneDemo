#include <memkind.h>

template<class T, MemoryType type>
class Array
{
public:
    Array(int num, MemoryPool& m)
    {
        size = num * sizeof(T);
        kind = m.poolPtr();
        if(type == DRAM)
            array = (T*) memkind_malloc(MEMKIND_DEFAULT, size);
        else
        {
            array = (T*) memkind_malloc(kind, size);
            printf("pmem!\n");
        }
    }
    ~Array()
    {
        memkind_free(kind, array);
    }
    struct memkind *kind;
    size_t size;
    T* array;
};
