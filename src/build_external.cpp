#include "build_external.h"

void build_external(std::string input, std::string output_file, int k,
                   int threads, std::string sequences,
                   std::vector<std::string> names) {
  if (sequences.size() * 2 + 1 < INT_MAX) {

    std::vector<int32_t> choped = quotient_external_32(
        sequences, k, std::filesystem::path(input).stem().string(), threads);
    auto it = std::max_element(choped.begin(), choped.end(), [](int a, int b) {
      return std::abs(a) < std::abs(b);
    });
    auto choped_nodes_number = std::abs(*it);

    std::cout << "Choped nodes number: " << choped_nodes_number << std::endl;
    std::cout << "Compacting unbranching paths..." << std::endl;

    // Path compression
    std::vector<uint8_t> states(choped_nodes_number + 1, 0);
    get_states(choped, sequences, states);
    std::vector<std::vector<int>> paths;
    std::vector<std::pair<int, int>> labels;
    collapse_paths<int, std::vector<int>>(states, choped, sequences, labels,
                                          paths);
    choped.clear();
    states.clear();
    // GFA writting
    std::cout << "Writing GFA..." << std::endl;
    write_gfa(labels, paths, output_file, sequences, names);

  } else {
    size_t choped_nodes_number = 0;
    signed_uint40_vector choped = quotient_external_40(
        sequences, k, choped_nodes_number,
        std::filesystem::path(input).stem().string(), threads);
    std::cout << "Choped nodes number: " << choped_nodes_number << std::endl;
    std::cout << "Compacting unbranching paths..." << std::endl;
    std::cout << choped.size() << "\n";
    // Path compression
    std::vector<uint8_t> states(choped_nodes_number + 1, 0);
    get_states(choped, sequences, states);
    std::cout << "/im here\n";
    std::vector<std::vector<int64_t>> paths;
    std::vector<std::pair<int64_t, int64_t>> labels;
    collapse_paths<int64_t, signed_uint40_vector>(states, choped, sequences,
                                                  labels, paths);
    choped.clear();
    states.clear();
    states.shrink_to_fit();
    // GFA writting
    std::cout << "Writing GFA..." << std::endl;
    write_gfa(labels, paths, output_file, sequences, names);
  }
}
