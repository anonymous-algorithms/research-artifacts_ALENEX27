#include "collapse_paths.hpp"
#include "fasta_utils.h"
#include "get_states.hpp"
#include "gfa_writer.hpp"
#include "hash_definitions.hpp"
#include "quotient_external.h"
#include "sequence_utils.h"
// #include <omp.h>
#include "build_external.h"
#include "build_rapid.h"
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
int main(int argc, char *argv[]) {
  using namespace std::chrono;

  std::string input;
  std::string output_file;
  int k = -1;
  int threads = 1;
  bool external = false;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "-i" && i + 1 < argc) {
      input = argv[++i];
    } else if (arg == "-o" && i + 1 < argc) {
      output_file = argv[++i];
    } else if (arg == "-t" && i + 1 < argc) {
      threads = std::stoi(argv[++i]);
    } else if (arg == "-k" && i + 1 < argc) {
      k = std::stoi(argv[++i]);
    } else if (arg == "-e") {
      external = true;
    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      return 1;
    }
  }

  if (input.empty() || output_file.empty() || k < 0) {
    std::cerr
        << "Usage: " << argv[0]
        << " -i <input_fasta> -o <output_gfa> -k <int> -t <threads> [-e]\n\n"
        << "Options:\n"
        << "  -i <file>       input FASTA file\n"
        << "  -o <file>       output GFA file\n"
        << "  -k <int>        k-mer size (integer)\n"
        << "  -t <int>        number of threads\n"
        << "  -e              external memory index construction\n";
    return 1;
  }

  // Read .fa file
  std::cout << "Reading fasta" << std::endl;
  std::vector<std::string> names;
  std::string sequences = read_sequences_from_fasta(input, names);
  // making sequence uppercse and replacing ambigious nt with N
  translate_sequence(sequences);

  auto total_length = sequences.size();
  std::cout << "Total length of sequences: " << total_length - names.size() - 1
            << "\n";
  std::cout << "Building structures ...\n";
  if (external) {
    build_external(input, output_file, k, threads, sequences, names);
  } else {
    build_rapid(input, output_file, k, threads, sequences, names);
  }
  return 0;
}
