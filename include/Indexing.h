#ifndef INDEXING
#define INDEXING
#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <omp.h>
#include "libsais.h"
#include "libsais64.h"

void build_phi(const std::string &T, std::vector<int32_t> &phi,
               const int threads = 1);
void build_phi(const std::string &T, std::vector<int64_t> &phi,
               const int threads = 1); 
#endif
