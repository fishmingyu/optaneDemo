#include <memkind.h>

template<class T>
class Array
{
public:
    Array(int num, MemoryPool& m)
    {
        size = num * sizeof(T);
        kind = m.poolPtr();
        if(m.mtype == DRAM)
            array = (T*) memkind_malloc(MEMKIND_DEFAULT, size);
        else
        {
            array = (T*) memkind_malloc(kind, size);
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
