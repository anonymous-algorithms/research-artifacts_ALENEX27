#include "build_rapid.h"

void build_rapid(std::string input, std::string output_file, int k, int threads, std::string sequences, std::vector<std::string> names){
std::vector<int> choped;
  if (sequences.size() * 2 + 1 < INT_MAX) {
    // Graph construction algorithm
    choped = quotient_rapid<int32_t>(sequences, k, threads);
    auto it = std::max_element(choped.begin(), choped.end(), [](int a, int b) {
      return std::abs(a) < std::abs(b);
    });
    auto choped_nodes_number = std::abs(*it);

    std::cout << "Choped nodes number: " << choped_nodes_number << std::endl;
    std::cout << "Compacting unbranching paths..." << std::endl;

    // Path compression
    std::vector<uint8_t> states(choped_nodes_number+1, 0);
    get_states(choped, sequences, states);
    std::cout << "States\n";
    std::vector<std::vector<int>> paths;
    std::vector<std::pair<int, int>> labels;
    collapse_paths_fast<int, int>(states, choped, sequences, labels, paths);
    choped.clear();
    states.clear();
    // GFA writting
    std::cout << "Writing GFA..." << std::endl;
    write_gfa(labels, paths, output_file, sequences, names);

  } else {
    std::vector<int64_t> choped = quotient_rapid<int64_t>(sequences, k, threads);
    auto it = std::max_element(choped.begin(), choped.end(), [](int a, int b) {
      return std::abs(a) < std::abs(b);
    });
    auto choped_nodes_number = std::abs(*it);

    std::cout << "Choped nodes number: " << choped_nodes_number << std::endl;
    std::cout << "Compacting unbranching paths..." << std::endl;

    // Path compression
    std::vector<uint8_t> states(choped_nodes_number+1, 0);
    get_states(choped, sequences, states);
    std::vector<std::vector<int64_t>> paths;
    std::vector<std::pair<int64_t, int64_t>> labels;
    collapse_paths_fast<int64_t, int64_t>(states, choped, sequences, labels, paths);
    choped.clear();
    states.clear();
    // GFA writting
    std::cout << "Writing GFA..." << std::endl;
    write_gfa(labels, paths, output_file, sequences, names);
  }
}

