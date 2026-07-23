#ifndef BUILD_RAPID
#define BUILD_RAPID

#include "quotient_rapid.hpp"
#include "collapse_paths.hpp"
#include "fasta_utils.h"
#include "get_states.hpp"
#include "gfa_writer.hpp"
#include "hash_definitions.hpp"
#include "sequence_utils.h"
#include "Indexing.h"
// #include <omp.h>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>




void build_rapid(std::string input, std::string output_file, int k, int threads, std::string sequences, std::vector<std::string> names);
#endif
