#include "memoryArray.h"
#include "memoryPool.h"
#include "spmm.h"
#include <args.hxx>
#include <limits.h>
#include <vector>

#include "./util/mmio.hpp"
#include "./util/util.hpp"

int main(int argc, char **argv) {
  args::ArgumentParser parser("Optane SPMM test.",
                              "This goes after the options.");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::ValueFlag<std::string> dax_path(parser, "dax_path", "Mapped dax file",
                                        {'d', "dax_path"});
  args::ValueFlag<int> memory_type(
      parser, "memory_type", "Memory type to use, 0 for DRAM, 1 for OPTANE",
      {'m', "memory_type"});
  args::ValueFlag<std::string> sparse_file(
      parser, "sparse_file", "Sparse matrix file path", {'s', "sparse_file"});
  args::CompletionFlag completion(parser, {"complete"});
  try {
    parser.ParseCLI(argc, argv);
  } catch (const args::Completion &e) {
    std::cout << e.what();
    return 0;
  } catch (const args::Help &) {
    std::cout << parser;
    return 0;
  } catch (const args::ParseError &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }
  std::string path = dax_path.Get();
  const MemoryType mytype = MemoryType(memory_type.Get());
  MemoryPool myOptane(path, mytype);
  int nrow;
  int ncol;
  int nnz;
  int kdim = 4;
  std::vector<int> row_CSR, row_COO, col_COO;
  std::vector<float> values_COO;
  readMtx<float>(sparse_file.Get().c_str(), row_COO, col_COO, values_COO, nrow,
                 ncol, nnz);
  COO_to_CSR(row_CSR, row_COO, nnz, nrow);
  Array<int> csrptr(nrow + 1, myOptane);
  Array<int> colind(nnz, myOptane);
  Array<float> values(nnz, myOptane);
  Array<float> dense(nrow * kdim, myOptane);
  Array<float> out(nrow * kdim, myOptane);

  srand(0);

  std::copy(row_CSR.begin(), row_CSR.end(), csrptr.array);
  std::copy(col_COO.begin(), col_COO.end(), colind.array);
  std::copy(values_COO.begin(), values_COO.end(), values.array);

  for (int i = 0; i < nrow * kdim; i++) {
    dense.array[i] = ((rand() % INT_MAX) / INT_MAX * 1.0);
  }
  uint64_t start = now();
  spmm(nrow, kdim, csrptr.array, colind.array, values.array, dense.array,
       out.array);
  uint64_t end = now();
  printf("finished time %ld us\n", end - start);
  return 0;
}
