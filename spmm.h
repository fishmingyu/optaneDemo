#ifndef SPMM_H
#define SPMM_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <vector>

unsigned long now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

void spmm(int nrow, int kdim, int *csrptr, int *colind, float *values,
          float *dense, float *out) {
  for (int i = 0; i < nrow; ++i) {
    for (int j = csrptr[i]; j < csrptr[i + 1]; ++j) {
      for (int k = 0; k < kdim; k++) {
        float val = values[j];
        int col = colind[j];
        out[i * kdim + k] += val * dense[col * kdim + k];
      }
    }
  }
}

void COO_to_CSR(std::vector<int> &row_CSR, std::vector<int> row_COO, int Size,
                int matrixRow) {
  row_CSR.push_back(0);
  if (row_COO[0] != 0) {
    for (int j = 0; j < row_COO[0]; j++)
      row_CSR.push_back(0);
  }
  for (int i = 0; i < (Size - 1); i++) {
    for (int j = 0; j < row_COO[i + 1] - row_COO[i]; j++) {
      row_CSR.push_back(i + 1);
    }
  }
  for (int j = 0; j < matrixRow + 1 - row_COO.back(); j++) {
    row_CSR.push_back(static_cast<int>(Size));
  }
}

#endif // SPMM_H