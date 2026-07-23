#ifndef PROCESS_BLOCK_IMPL_HPP
#define PROCESS_BLOCK_IMPL_HPP
#include<iostream>
#include <vector>
#include <cmath> 
#include <cstdlib>
#include <cstddef>


//template <typename T1, typename T2>
//void process_block(
//    robin_hood::unordered_node_map<robin_hood::pair<T1, T1>, T1> &nodes_id,
//    std::vector<std::pair<T1, T1>> &nodes_labels, std::vector<T1> &path,
//    T1 start, T1 stop, T1 &x, const std::string &sequence,
//    const T2 &transformed) {
 

template <typename T1, typename T2>
void process_block(std::vector<T1> &nodes_id,
                   std::vector<std::pair<T1, T1>> &nodes_labels,
                   std::vector<T1> &path, size_t start, size_t stop, T1 &x,
                   const T2 &transformed) {

  constexpr auto shift = 8*sizeof(T1)-1;
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
