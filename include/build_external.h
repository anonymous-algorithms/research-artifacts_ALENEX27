#ifndef BUILD_EXTERNAL
#define BUILD_EXTERNAL
#include "collapse_paths.hpp"
#include "fasta_utils.h"
#include "get_states.hpp"
#include "gfa_writer.hpp"
#include "hash_definitions.hpp"
#include "quotient_external.h"
#include "sequence_utils.h"
// #include <omp.h>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

void build_external(std::string input, std::string output_file, int k,
                   int threads, std::string sequences,
                   std::vector<std::string> names);
#endif
