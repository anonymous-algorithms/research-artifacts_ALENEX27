#ifndef QUOTIENT_RAPID
#define QUOTIENT_RAPID

#include <bits/stdc++.h>
#include "sequence_utils.h"
#include <chrono>
#include "Indexing.h"
#include "dsu.hpp"
#include "union_kmers.hpp"


template <typename T1>
std::vector<T1> quotient_rapid(std::string &seq, int k, int threads = 1) {

  using clock = std::chrono::high_resolution_clock;
  auto t1 = clock::now();
  auto n = seq.size();
  // Dodajemy reverse complement
  std::cout << "Building reverse complement...\n";
  rc_inplace(seq, threads);
  seq.push_back('\0');
	std::vector<T1> phi;
  build_phi(seq, phi, threads);
  std::vector<T1> sentinels = find_sentinels<T1>(seq);

  DSU<T1> UF;
  UF.init(T1(n));
  auto t2 = clock::now();
  auto dt1 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  std::cout << "Index construction time: " << dt1.count() << "ms\n";
  std::cout << "Constructing canonical graph...\n";
  t1 = clock::now();
  union_kmers(phi, k, UF, n, sentinels, seq.data());
  t2 = clock::now();
  auto dt2 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

  std::cout << "Graph construction main step time: " << dt2.count() << "ms\n";

  seq.resize(n); 
  std::string tmp;
  seq.swap(tmp);
  seq = std::move(tmp);
  phi.clear();
  phi.shrink_to_fit();

  std::cout << "Enumerating and orienting nodes...\n";
  auto cls = compute_class_ids_vector<T1>(UF, seq, n);
  return cls;
}
#endif
