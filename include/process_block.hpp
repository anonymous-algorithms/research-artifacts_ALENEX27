#ifndef PROCESS_BLOCK_IMPL_HPP
#define PROCESS_BLOCK_IMPL_HPP

#include "../externals/robin-hood-hashing/src/include/robin_hood.h"
#include <string>
#include <vector>

template <typename T1, typename T2>
void process_block(
    robin_hood::unordered_node_map<robin_hood::pair<T1, T1>, T1> &nodes_id,
    std::vector<std::pair<T1, T1>> &nodes_labels, std::vector<T1> &path,
    T1 start, T1 stop, T1 &x, const std::string &sequence,
    const T2 &transformed) {
  robin_hood::pair<T1, T1> n(transformed[start], transformed[stop]);
  robin_hood::pair<T1, T1> m(-n.second, -n.first);
  if (nodes_id.find(n) != nodes_id.end()) {
    path.push_back(nodes_id[n]);
  } else if (nodes_id.find(m) != nodes_id.end()) {
    path.push_back(-1 * nodes_id[m]);
  } else {
    nodes_id[n] = x;
    nodes_labels.push_back({start, stop + 1 - start});
    path.push_back(nodes_id[n]);
    x += 1;
  }
}

template <typename T1, typename T2>
void process_block_fast(std::vector<T2> &nodes_id,
                   std::vector<std::pair<T1, T1>> &nodes_labels,
                   std::vector<T2> &path, size_t start, size_t stop, T2 &x,
                   const std::vector<T2> &transformed) {

  constexpr auto shift = 8*sizeof(T2)-1;
  auto n = std::abs(transformed[start]);
  auto m = std::abs(transformed[stop]);
  auto previous_value = nodes_id[n];
  if (start != stop) {
    if (previous_value != 0) {
      path.push_back(previous_value);
    } else {
      nodes_id[n] = x;
      nodes_id[m] = -x;
      nodes_labels.push_back({start, stop + 1 - start});
      path.push_back(x);
      x += 1;
    }
  } else {

		    if (previous_value != 0) {
			
      if ((1 | (transformed[start] >> shift)) * (1 | (previous_value >> shift))>0) {
        path.push_back(std::abs(previous_value));
      }
      else {
        path.push_back(-std::abs(
            previous_value));
      }
    } else {
			nodes_id[n] =
          x -
          2 * x * (transformed[start] < 0);
      nodes_labels.push_back({start, stop + 1 - start});
      path.push_back(x);
      x += 1;
    }
  }
}



#endif // PROCESS_BLOCK_IMPL_HPP
