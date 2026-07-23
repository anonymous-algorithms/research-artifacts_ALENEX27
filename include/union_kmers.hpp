#ifndef UNION_KMERS
#define UNION_KMERS

#include "build_samples.h"
#include "dsu.hpp"

template <typename T1> inline T1 map_rev_to_fwd(T1 pos, size_t orig_len) {
  return 2 * orig_len - pos;
}

template <typename T1> struct SequenceMapper {
  std::vector<T1> seq_starts; // początki sekwencji w tekście

  SequenceMapper(const std::vector<T1> &starts) : seq_starts(starts) {
    sort(seq_starts.begin(), seq_starts.end());
  }

  int get_sequence_id(T1 pos) const {
    auto it = upper_bound(seq_starts.begin(), seq_starts.end(), pos);
    if (it == seq_starts.begin())
      return -1;
    return distance(seq_starts.begin(), it - 1);
  }
};

template <typename T1>
void union_kmers(const std::vector<T1> &phi, int k, DSU<T1> &UF,
                 size_t orig_len, const std::vector<T1> &sentinels,
                 const char *text) {
  T1 n = (T1)phi.size();
  T1 prev_skip = 0;
  T1 h = 0;
  T1 lcp;
  size_t sentinel_idx = 0;
  for (T1 posA = 1; posA < n - 1; posA++) {
    if (posA == sentinels[sentinel_idx]) {
      h = 0;
      continue;
    }
    T1 posB = phi[posA];
    while (sentinel_idx < sentinels.size() && sentinels[sentinel_idx] <= posA) {
      sentinel_idx++;
    }
    if (sentinel_idx >= sentinels.size())
      break; // koniec tekstu
    T1 dist_to_sentinel = sentinels[sentinel_idx] - posA;
    const char *pa = text + posA;
    const char *pb = text + posB;
    while (pa[h] == pb[h]) 
    {
      ++h;
      }
    lcp = h;
    T1 cur_LCP = std::min(lcp, dist_to_sentinel);
    T1 skip = (prev_skip - 1) * (prev_skip > k);
    if (cur_LCP >= k) {
      for (T1 m = skip; m < cur_LCP; m++) {
        T1 a = posA + m;
        T1 b = posB + m;
        // mapowanie reverse complement → oryginał
        if (a >= orig_len)
          a = map_rev_to_fwd(a, orig_len);
        if (b >= orig_len)
          b = map_rev_to_fwd(b, orig_len);
        UF.unite(a, b);
      }
      prev_skip = cur_LCP;
    } else {
      prev_skip = 0;
    }
    h -= (h > 0);
  }
}

template <typename T1>
std::vector<T1> compute_class_ids_vector(DSU<T1> &UF, const std::string &T,
                                         size_t orig_len) {
  std::vector<T1> class_ids(UF.parent.size(), 0); // zerowy = nieprzypisany
  size_t idx = 1;
  for (T1 pos = 1; pos < orig_len; pos++) {
    if (T[pos] == '$')
      continue;

    T1 root = UF.find(pos);

    if (class_ids[root] == 0) {
      class_ids[root] = idx++;
    }

    class_ids[pos] =
        class_ids[root] - 2 * (T[pos] != T[root]) * class_ids[root];
  }

  return class_ids;
}

template <typename T1> std::vector<T1> find_sentinels(const std::string &T) {
  std::vector<T1> positions;
  for (size_t i = 0; i < T.size(); i++) {
    if (T[i] == '$') {
      positions.push_back((T1)i);
    }
  }
  return positions;
}

template <typename T1>
void union_kmers_streamed(const char *samples_file, int k, DSU<T1> &UF,
                          size_t orig_len, const char *text) {
  const size_t CHUNK = 1 << 16; //
  const size_t MAX_CHUNKS = 4;
  std::queue<Chunk> q;
  std::mutex mtx;
  std::condition_variable cv;
  bool done = false;
  // ======================
  // PRODUCER
  // ======================
  std::thread producer([&]() {
    FILE *f = fopen(samples_file, "rb");
    if (!f)
      throw std::runtime_error("Cannot open samples file");
    size_t idx = 0;
    std::vector<Sample_PLCP> buffer(CHUNK);
    while (true) {
      size_t read = fread(buffer.data(), sizeof(Sample_PLCP), CHUNK, f);
      if (read == 0)
        break;
      Chunk chunk;
      chunk.data.resize(read);
      chunk.start_idx = idx;
      memcpy(chunk.data.data(), buffer.data(), read * sizeof(Sample_PLCP));
      {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return q.size() < MAX_CHUNKS; });
        q.push(std::move(chunk));
      }
      cv.notify_all();
      idx += read;
    }
    {
      std::unique_lock<std::mutex> lock(mtx);
      done = true;
    }
    cv.notify_one();
    fclose(f);
  });
  // ======================
  // CONSUMER (main loop)
  // ======================
  T1 prev_skip = 0;
  size_t h = 0;

  while (true) {
    Chunk chunk;
    {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [&]() { return !q.empty() || done; });
      if (q.empty() && done)
        break;
      chunk = std::move(q.front());
      q.pop();
      cv.notify_all();
    }
    const std::vector<Sample_PLCP> &phi = chunk.data;
    for (size_t local = 0; local < phi.size(); ++local) {
      T1 posA = phi[local].first;
      T1 posB = phi[local].second;
      uint64_t h = static_cast<uint64_t>(phi[local].third);
      if (h >= k) {
        for (size_t m = 0; m < h; m++) {
          size_t a = posA + m;
          size_t b = posB + m;
          if (a >= orig_len)
            a = map_rev_to_fwd(a, orig_len);
          if (b >= orig_len)
            b = map_rev_to_fwd(b, orig_len);
          UF.unite(a, b);
        }
      }
    }
  }

  producer.join();
}
#endif
