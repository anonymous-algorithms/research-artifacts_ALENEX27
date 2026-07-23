#ifndef QUOTIENT_E
#define QUOTIENT_E

#include <bits/stdc++.h>
#include <divsufsort.h> // libdivsufsort
#include "sequence_utils.h"
#include <ankerl/unordered_dense.h>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <stdexcept>
#include "uint40.hpp"
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <cassert>
#include <utility>
#include <vector>
#include "build_samples.h"
#include "union_kmers.hpp"
#include "dsu.hpp"
#include "build_samples.h"



//template <typename T1>
std::vector<int32_t> quotient_external_32(std::string &seq, int k,std::string input_name,  int threads = 1);
signed_uint40_vector quotient_external_40(std::string &seq, int k, size_t &v,std::string input_name, int threads = 1);
#endif
