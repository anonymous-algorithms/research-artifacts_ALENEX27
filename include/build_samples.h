#ifndef SAMPLE
#define SAMPLE

#include "uint40.hpp"
#include<vector>
#include<cstdint>
#include <bits/stdc++.h>


struct signed_uint40_vector {
  std::vector<uint40> values;
  std::vector<uint64_t> bitvector;
  signed_uint40_vector(std::vector<uint40> &&values_,
                       std::vector<uint64_t> &&bitvector_)
      : values(std::move(values_)), bitvector(std::move(bitvector_)) {
    assert(bitvector.size() >= (values.size() + 63) / 64);
  }

  inline size_t size() const { return values.size(); }
  inline void clear() {
    std::vector<uint40>().swap(values);
    std::vector<uint64_t>().swap(bitvector);
  }
  inline int64_t operator[](size_t i) const {
    //assert(i < values.size());

    const int64_t x = static_cast<uint64_t>(values[i]);
    const bool positive = (bitvector[i >> 6] >> (i & 63)) & 1ULL;

    return positive ? x : -x;
  }
};

struct Sample_PLCP {
  uint40 first;
  uint40 second;
  uint40 third;
} __attribute__((packed));

struct Chunk {
	std::vector<Sample_PLCP> data;
  size_t start_idx; // globalny offset w samples
  bool last = false;
};

void build_samples(const std::string &sa_filename,
                    const std::string &samples_filename, const char *T,
                    uint64_t n,
                    size_t max_chunk_bytes = 64ULL << 20,  // np. 1 GiB
                    size_t sample_buf_bytes = 64ULL << 20); // np. 64 MiB




#endif
