#include <memkind.h>

char realPath[PATH_MAX] = "/daxfs";
class MemoryPool
{
private:
    struct memkind *pmem_kind;
public:
    MemoryPool(const char* path, int poolSize);
    struct memkind* poolPtr();
    ~MemoryPool();
};

MemoryPool::MemoryPool(const char* p, int poolSize = 0)
{
    if(realpath(p, realPath) == NULL)
    {
        perror(p);
    }
    int err = memkind_create_pmem(p, poolSize, &pmem_kind);
    if (err) {
        memkind_fatal(err);
    }
}

struct memkind* MemoryPool::poolPtr()
{
    return pmem_kind;
}

MemoryPool::~MemoryPool()
{
    int err = memkind_destroy_kind(pmem_kind);
	if (err) {
		memkind_fatal(err);
	}
}
