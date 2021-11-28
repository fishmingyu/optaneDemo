// #include <iostream>
#include "./util/mmio.hpp"
#include "./util/util.hpp"
#include "spmm.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#define ITER 200

// dense [nrow, kdim]
// out [nrow, kdim]

template <class T> class alignas(32) DramArray {
public:
  DramArray(int num) {
    size = num * sizeof(T);
    array = (T *)malloc(size);
  }
  void reset() { memset(array, 0, sizeof(T) * size); }
  ~DramArray() { free(array); }
  size_t size;
  T *array;
};

int main(int argc, char **argv) {
  int nrow;
  int ncol;
  int nnz;
  int kdim = atoi(argv[2]);
  std::vector<int> row_CSR, row_COO, col_COO;
  std::vector<float> values_COO;
  readMtx<float>(argv[1], row_COO, col_COO, values_COO, nrow, ncol, nnz);
  COO_to_CSR(row_CSR, row_COO, nnz, nrow);

  DramArray<int> csrptr(nrow + 1);
  DramArray<int> colind(nnz);
  DramArray<float> values(nnz);
  DramArray<float> dense(nrow * kdim);
  DramArray<float> out1(nrow * kdim);
  DramArray<float> out2(nrow * kdim);

  std::copy(row_CSR.begin(), row_CSR.end(), csrptr.array);
  std::copy(col_COO.begin(), col_COO.end(), colind.array);
  std::copy(values_COO.begin(), values_COO.end(), values.array);

  srand(0);
  for (int i = 0; i < nrow * kdim; i++) {
    dense.array[i] = ((rand() % INT_MAX) * 1.0 / INT_MAX);
  }
  // warp up
  spmm_omp32(nrow, kdim, csrptr.array, colind.array, values.array, dense.array,
             out1.array);
  spmm(nrow, kdim, csrptr.array, colind.array, values.array, dense.array,
       out2.array);

  for (int i = 0; i < 100; i++)
    if (out1.array[i] - out2.array[i] > 1e-5) {
      printf("mistake %d %f, %f\n", i, out1.array[i], out2.array[i]);
      // break;
    }
  unsigned long start = now();
  for (int i = 0; i < ITER; i++)
    spmm_omp32(nrow, kdim, csrptr.array, colind.array, values.array,
               dense.array, out1.array);
  unsigned long end = now();
  printf("omp spmm finished %f s \n", (end - start) / 1e6 / ITER);
  start = now();
  for (int i = 0; i < ITER; i++)
    spmm(nrow, kdim, csrptr.array, colind.array, values.array, dense.array,
         out2.array);
  end = now();
  printf("serial spmm finished %f s \n", (end - start) / 1e6 / ITER);
  return 0;
}