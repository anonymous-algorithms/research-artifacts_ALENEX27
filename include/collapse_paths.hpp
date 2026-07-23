#ifndef COLLAPSE_PATHS_IMPL_HPP
#define COLLAPSE_PATHS_IMPL_HPP

#include "../externals/robin-hood-hashing/src/include/robin_hood.h"
#include "process_block.hpp"
#include <string>
#include <vector>

template <typename T1, typename T2>
void collapse_paths(const std::vector<uint8_t> &states,
                    T2 &transformed,
                    const std::string &sequence,
                    std::vector<std::pair<T1, T1>> &nodes_labels,
                    std::vector<std::vector<T1>> &paths) {

  robin_hood::unordered_node_map<robin_hood::pair<T1, T1>, T1> nodes_id;
  T1 start = 1;
  T1 x = 1;
  std::vector<T1> path;
  for (T1 i = 1; i < transformed.size(); i++) {

    if ((states[abs(transformed[i])] == 1 && transformed[i] > 0) ||
        (states[abs(transformed[i])] == 2 &&
         transformed[i] < 0)) // BRANCH ON LEFT
    {
      if (i == start) {
        continue;
      }
      process_block(nodes_id, nodes_labels, path, start, i - 1, x, sequence,
                    transformed);
      start = i;

    }

    else if ((states[abs(transformed[i])] == 1 && transformed[i] < 0) ||
             (states[abs(transformed[i])] == 2 &&
              transformed[i] > 0)) // BRANCH ON RIGHT
    {
      process_block(nodes_id, nodes_labels, path, start, i, x, sequence,
                    transformed);
      start = i + 1;
    } else if (states[abs(transformed[i])] == 3) // DOUBLE SIDE
    {
      if (i != start) {
        process_block(nodes_id, nodes_labels, path, start, i - 1, x, sequence,
                      transformed);
      }
      process_block(nodes_id, nodes_labels, path, i, i, x, sequence,
                    transformed);
      start = i + 1;
    }

    if (transformed[i] == 0) // ADD LAST BLOCK
    {
      if (start < i) {

        process_block(nodes_id, nodes_labels, path, start, i - 1, x, sequence,
                      transformed);
      }
      paths.push_back(path);
      path.clear();
      start = i + 1;
    }
  }
}
template <typename T1, typename T2>
void collapse_paths_fast(const std::vector<uint8_t> &states,
                    const std::vector<T2> &transformed,
                    const std::string &sequence,
                    std::vector<std::pair<T1, T1>> &nodes_labels,
                    std::vector<std::vector<T2>> &paths) {
  std::vector<T2> nodes_id(states.size(), 0);

  size_t start = 1;
  T2 x = 1;
  std::vector<T2> path;
  for (size_t i = 1; i < transformed.size(); i++) {
    auto state = states[abs(transformed[i])];
    bool positive = transformed[i] > 0;

    if ((state == 1 && positive) || (state == 2 && !positive)) // BRANCH ON LEFT
    {
      if (i == start) {
        continue;
      }
      process_block_fast(nodes_id, nodes_labels, path, start, i - 1, x, transformed);
      start = i;
    }

    else if ((state == 1 && !positive) ||
             (state == 2 && positive)) // BRANCH ON RIGHT
    {
      process_block_fast(nodes_id, nodes_labels, path, start, i, x, transformed);
      start = i + 1;
    } else if (state == 3) // DOUBLE SIDE
    {
      if (i != start) {
        process_block_fast(nodes_id, nodes_labels, path, start, i - 1, x,
                      transformed);
      }
      process_block_fast(nodes_id, nodes_labels, path, i, i, x, transformed);
      start = i + 1;
    }

    if (transformed[i] == 0) // ADD LAST BLOCK
    {
      if (start < i) {
        process_block_fast(nodes_id, nodes_labels, path, start, i - 1, x,
                      transformed);
      }
      paths.push_back(path);
      path.clear();
      start = i + 1;
    }
  }
}


#endif // COLLAPSE_PATHS_IMPL_HPP
