#ifndef COLLAPSE_PATHS_IMPL_HPP
#define COLLAPSE_PATHS_IMPL_HPP

#include "process_block.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>


//void collapse_paths(const std::vector<uint8_t> &states,
//                    T2 &transformed,
//                    const std::string &sequence,
//                    std::vector<std::pair<T1, T1>> &nodes_labels,
//                    std::vector<std::vector<T1>> &paths) {



template <typename T1, typename T2>
void collapse_paths(const std::vector<uint8_t> &states,
                    const T2 &transformed,
                    const std::string &sequence,
                    std::vector<std::pair<T1, T1>> &nodes_labels,
                    std::vector<std::vector<T1>> &paths) {
  std::vector<T1> nodes_id(states.size(), 0);

  size_t start = 1;
  T1 x = 1;
  std::vector<T1> path;
  for (size_t i = 1; i < transformed.size(); i++) {
    auto state = states[abs(transformed[i])];
    bool positive = transformed[i] > 0;

    if ((state == 1 && positive) || (state == 2 && !positive)) // BRANCH ON LEFT
    {
      if (i == start) {
        continue;
      }
      process_block(nodes_id, nodes_labels, path, start, i - 1, x, transformed);
      start = i;
    }

    else if ((state == 1 && !positive) ||
             (state == 2 && positive)) // BRANCH ON RIGHT
    {
      process_block(nodes_id, nodes_labels, path, start, i, x, transformed);
      start = i + 1;
    } else if (state == 3) // DOUBLE SIDE
    {
      if (i != start) {
        process_block(nodes_id, nodes_labels, path, start, i - 1, x,
                      transformed);
      }
      process_block(nodes_id, nodes_labels, path, i, i, x, transformed);
      start = i + 1;
    }

    if (transformed[i] == 0) // ADD LAST BLOCK
    {
      if (start < i) {
        process_block(nodes_id, nodes_labels, path, start, i - 1, x,
                      transformed);
      }
      paths.push_back(std::move(path));
      //path.clear();
      start = i + 1;
    }
  }
}

#endif // COLLAPSE_PATHS_IMPL_HPP
