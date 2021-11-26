// #include <iostream>
#include "./util/mmio.hpp"
#include "./util/util.hpp"
#include "spmm.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// dense [nrow, kdim]
// out [nrow, kdim]

template <class T> class DramArray {
public:
  DramArray(int num) {
    size = num * sizeof(T);
    array = (T *)malloc(size);
  }
  ~DramArray() { free(array); }
  size_t size;
  T *array;
};

int main(int argc, char **argv) {
  int nrow;
  int ncol;
  int nnz;
  int kdim = 4;
  std::vector<int> row_CSR, row_COO, col_COO;
  std::vector<float> values_COO;
  readMtx<float>(argv[1], row_COO, col_COO, values_COO, nrow, ncol, nnz);
  COO_to_CSR(row_CSR, row_COO, nnz, nrow);

  DramArray<int> csrptr(nrow + 1);
  DramArray<int> colind(nnz);
  DramArray<float> values(nnz);
  DramArray<float> dense(nrow * kdim);
  DramArray<float> out(nrow * kdim);

  std::copy(row_CSR.begin(), row_CSR.end(), csrptr.array);
  std::copy(col_COO.begin(), col_COO.end(), colind.array);
  std::copy(values_COO.begin(), values_COO.end(), values.array);

  srand(0);
  for (int i = 0; i < nrow * kdim; i++) {
    dense.array[i] = ((rand() % INT_MAX) / INT_MAX * 1.0);
  }
  unsigned long start = now();
  spmm(nrow, kdim, csrptr.array, colind.array, values.array, dense.array,
       out.array);
  unsigned long end = now();
  printf("finished %ld\n us", end - start);
  return 0;
}