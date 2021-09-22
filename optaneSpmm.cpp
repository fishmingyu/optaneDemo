#include "spmm.h"
#include <limits.h>
#include <vector>
#include "optaneUtil.h"
#include "memoryPool.h"
#include "memoryArray.h"

#include "./util/mmio.hpp"
#include "./util/util.hpp"

int main(int argc, char** argv)
{
    MemoryPool myOptane(argv[1]);
    const MemoryType mytype = MemoryType::DRAM;
    int nrow;
    int ncol;
    int nnz;
    int kdim = 4;
    std::vector<int> row_CSR, row_COO, col_COO;
    std::vector<float> values_COO;
    readMtx<float>(argv[2], row_COO, col_COO, values_COO, nrow, ncol, nnz);
    COO_to_CSR(row_CSR, row_COO, nnz, nrow);
    Array<int, mytype> csrptr(nrow, myOptane);
    Array<int, mytype> colind(nnz, myOptane);
    Array<float, mytype> values(nnz, myOptane);
    Array<float, mytype> dense(nrow * kdim, myOptane);
    Array<float, mytype> out(nrow * kdim, myOptane);

    std::copy(row_CSR.begin(), row_CSR.end(), csrptr.array);
    std::copy(col_COO.begin(), col_COO.end(), colind.array);
    std::copy(values_COO.begin(), values_COO.end(), values.array);
    
    srand(0);
    for(int i = 0;i< nrow * kdim;i++)
    {
        out.array[i] = ((rand() % INT_MAX) / INT_MAX * 1.0);
        dense.array[i] = ((rand() % INT_MAX) / INT_MAX * 1.0);
    }
    uint64_t start = now();
    spmm(nrow, kdim, csrptr.array, colind.array, values.array, dense.array, out.array);
    uint64_t end = now();
    printf("finished time %ld us\n", end - start);
    return 0;
}
