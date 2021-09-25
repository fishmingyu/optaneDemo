#include <memkind.h>
#include <string>
#include "optaneUtil.h"
char realPath[PATH_MAX] = "/pmem1";
class MemoryPool
{
private:
    struct memkind *pmem_kind;
public:
    MemoryPool(std::string path, MemoryType mt, int poolSize);
    struct memkind* poolPtr();
    enum MemoryType mtype;
    ~MemoryPool();
};

MemoryPool::MemoryPool(std::string p, MemoryType mt, int poolSize = 0)
{
    if(realpath(p.c_str(), realPath) == NULL)
    {
        perror(p.c_str());
    }
    int err = memkind_create_pmem(p.c_str(), poolSize, &pmem_kind);
    if (err) {
        memkind_fatal(err);
    }
    mtype = mt;
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
