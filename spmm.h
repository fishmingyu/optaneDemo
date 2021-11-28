#ifndef SPMM_H
#define SPMM_H

#include "omp.h"
#include <immintrin.h>
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
    int lb = csrptr[i];
    int hb = csrptr[i + 1];
    for (int j = lb; j < hb; ++j) {
      float val = values[j];
      int col = colind[j];
      for (int k = 0; k < kdim; k++) {
        out[i * kdim + k] += val * dense[col * kdim + k];
      }
    }
  }
}

void spmm_omp(int nrow, int kdim, int *csrptr, int *colind, float *values,
              float *dense, float *out) {
  int i, j, k;
#pragma omp parallel for num_threads(24) private(i, k, j)
  for (i = 0; i < nrow; ++i) {
    int lb = csrptr[i];
    int hb = csrptr[i + 1];
    int ktmp = kdim - (kdim % 8);
#pragma unroll 4
    // #pragma omp parallel for
    for (k = 0; k < (kdim / 8); k++) {
      __m256 valc = _mm256_setzero_ps();
      for (j = lb; j < hb; ++j) {
        int col = colind[j];
        __m256 valb = _mm256_loadu_ps(dense + col * kdim + k * 8);
        float val = values[j];
        __m256 vala = _mm256_broadcast_ss(&val);
        valc = _mm256_fmadd_ps(vala, valb, valc);
      }
      _mm256_storeu_ps(out + i * kdim + k * 8, valc);
    }
#pragma unroll 4
    for (int k = ktmp; k < kdim; k++) {
      float acc = 0;
      for (int j = lb; j < hb; ++j) {
        float val = values[j];
        int col = colind[j];
        acc += val * dense[col * kdim + k];
      }
      out[i * kdim + k] = acc;
    }
  }
}

void spmm_omp16(int nrow, int kdim, int *csrptr, int *colind, float *values,
                float *dense, float *out) {
  int i, j, k;
#pragma omp parallel for num_threads(24) private(i, k, j)
  for (i = 0; i < nrow; ++i) {
    int lb = csrptr[i];
    int hb = csrptr[i + 1];
    int ktmp16 = kdim - (kdim % 16);
    int ktmp8 = kdim - (kdim % 8);
#pragma unroll 4
    // #pragma omp parallel for
    for (k = 0; k < (kdim / 16); k++) {
      __m512 valc = _mm512_setzero_ps();
      for (j = lb; j < hb; ++j) {
        int col = colind[j];
        float val = values[j];
        __m512 valb = _mm512_loadu_ps(dense + col * kdim + k * 16);
        __m512 vala = _mm512_set1_ps(val);
        valc = _mm512_fmadd_ps(vala, valb, valc);
      }
      _mm512_storeu_ps(out + i * kdim + k * 16, valc);
    }
    for (int k = ktmp16; k < ktmp8; k++) {
      __m256 valc = _mm256_setzero_ps();
      for (j = lb; j < hb; ++j) {
        int col = colind[j];
        __m256 valb = _mm256_loadu_ps(dense + col * kdim + ktmp16);
        float val = values[j];
        __m256 vala = _mm256_broadcast_ss(&val);
        valc = _mm256_fmadd_ps(vala, valb, valc);
      }
      _mm256_storeu_ps(out + i * kdim + ktmp16, valc);
    }
#pragma unroll 4
    for (int k = ktmp8; k < kdim; k++) {
      float acc = 0;
      for (int j = lb; j < hb; ++j) {
        float val = values[j];
        int col = colind[j];
        acc += val * dense[col * kdim + k];
      }
      out[i * kdim + k] = acc;
    }
  }
}

void spmm_omp32(int nrow, int kdim, int *csrptr, int *colind, float *values,
                float *dense, float *out) {
  int i, j, k;
#pragma omp parallel for num_threads(24) private(i, k, j)
  for (i = 0; i < nrow; ++i) {
    int lb = csrptr[i];
    int hb = csrptr[i + 1];
    int ktmp32 = kdim - (kdim % 32);
    int ktmp16 = kdim - (kdim % 16);
    int ktmp8 = kdim - (kdim % 8);
#pragma unroll 4
    // #pragma omp parallel for
    for (k = 0; k < (kdim / 32); k++) {
      __m512 valc1 = _mm512_setzero_ps();
      __m512 valc2 = _mm512_setzero_ps();
      for (j = lb; j < hb; ++j) {
        int col = colind[j];
        float val = values[j];
        __m512 valb1 = _mm512_loadu_ps(dense + col * kdim + k * 32);
        __m512 valb2 = _mm512_loadu_ps(dense + col * kdim + k * 32 + 16);
        __m512 vala = _mm512_set1_ps(val);
        valc1 = _mm512_fmadd_ps(vala, valb1, valc1);
        valc2 = _mm512_fmadd_ps(vala, valb2, valc2);
      }
      _mm512_storeu_ps(out + i * kdim + k * 32, valc1);
      _mm512_storeu_ps(out + i * kdim + k * 32 + 16, valc2);
    }
    for (k = ktmp32; k < ktmp16; k += 16) {
      __m512 valc = _mm512_setzero_ps();
      for (j = lb; j < hb; ++j) {
        int col = colind[j];
        float val = values[j];
        __m512 valb = _mm512_loadu_ps(dense + col * kdim + ktmp32 + k);
        __m512 vala = _mm512_set1_ps(val);
        valc = _mm512_fmadd_ps(vala, valb, valc);
      }
      _mm512_storeu_ps(out + i * kdim + ktmp32 + k, valc);
    }
    for (int k = ktmp16; k < ktmp8; k += 8) {
      __m256 valc = _mm256_setzero_ps();
      for (j = lb; j < hb; ++j) {
        int col = colind[j];
        float val = values[j];
        __m256 valb = _mm256_loadu_ps(dense + col * kdim + ktmp16 + k);
        __m256 vala = _mm256_broadcast_ss(&val);
        valc = _mm256_fmadd_ps(vala, valb, valc);
      }
      _mm256_storeu_ps(out + i * kdim + ktmp16 + k, valc);
    }
#pragma unroll 4
    for (int k = ktmp8; k < kdim; k++) {
      float acc = 0;
      for (int j = lb; j < hb; ++j) {
        float val = values[j];
        int col = colind[j];
        acc += val * dense[col * kdim + k];
      }
      out[i * kdim + k] = acc;
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