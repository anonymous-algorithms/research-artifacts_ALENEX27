#ifndef DSU_HPP
#define DSU_HPP

#include<vector>
#include<cstdint>

// ===== Union-Find =====
template <typename T1> 
struct DSU {
	std::vector<T1> parent;
	std::vector<uint8_t> rank;
  void init(size_t n) {
    parent.resize(n);
    rank.assign(n, 0);
    iota(parent.begin(), parent.end(), T1(0));
  }
  T1 find(T1 x) { return parent[x] == x ? x : parent[x] = find(parent[x]); }
  void unite(T1 a, T1 b) {
    a = find(a);
    b = find(b);
    if (a == b)
      return;
    if (rank[a] < rank[b])
      std::swap(a, b);
    parent[b] = a;
    if (rank[a] == rank[b])
      rank[a]++;
  }
};
#endif
