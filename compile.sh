g++ -std=c++11 -o dramSpmm dramSpmm.cc -fopenmp -mavx -mavx2 -mfma -mavx512f
g++ -std=c++11 -o optaneSpmm optaneSpmm.cpp -lmemkind