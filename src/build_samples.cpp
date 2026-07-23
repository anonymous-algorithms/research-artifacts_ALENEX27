#include "build_samples.h" 


void build_samples(const std::string &sa_filename,
                    const std::string &samples_filename, const char *T,
                    uint64_t n,
                    size_t max_chunk_bytes,  // np. 1 GiB
                    size_t sample_buf_bytes) // np. 64 MiB
{
  std::ifstream sa_in(sa_filename, std::ios::binary);
  assert(sa_in);

  std::ofstream sample_out(samples_filename, std::ios::binary);
  assert(sample_out);
  const size_t sa_chunk_size =
      std::max<size_t>(1, max_chunk_bytes / sizeof(uint40));

  const size_t sample_buf_size =
      std::max<size_t>(1, sample_buf_bytes / sizeof(Sample_PLCP));

  std::vector<uint40> sa_buf(sa_chunk_size);
  std::vector<Sample_PLCP> sample_buf;
  sample_buf.reserve(sample_buf_size);

  uint64_t processed = 0;

  uint40 prev_sa;
  bool first = true;

  while (true) {
    sa_in.read(reinterpret_cast<char *>(sa_buf.data()),
               sa_chunk_size * sizeof(uint40));

    size_t elems = sa_in.gcount() / sizeof(uint40);
    if (elems == 0)
      break;

    for (size_t j = 0; j < elems; ++j, ++processed) {
      uint40 cur_sa = sa_buf[j];

      if (first) {
        prev_sa = cur_sa;
        first = false;
        continue;
      }

      uint64_t cur = static_cast<uint64_t>(cur_sa);
      uint64_t prev = static_cast<uint64_t>(prev_sa);

      char bwt_char1 = T[(cur + n - 1) % n];
      char bwt_char2 = T[(prev + n - 1) % n];
      uint64_t h = 0;
      if (bwt_char1 != bwt_char2 || bwt_char1 == '$') {
        while (T[cur + h] == T[prev + h] && T[cur + h] != '$') {
          h++;
        }
        sample_buf.push_back({cur_sa, prev_sa, uint40(h)});
        h = 0;
        if (sample_buf.size() == sample_buf_size) {
          sample_out.write(reinterpret_cast<const char *>(sample_buf.data()),
                           sample_buf.size() * sizeof(Sample_PLCP));
          sample_buf.clear();
        }
      }

      prev_sa = cur_sa;
    }
  }

  if (!sample_buf.empty()) {
    sample_out.write(reinterpret_cast<const char *>(sample_buf.data()),
                     sample_buf.size() * sizeof(Sample_PLCP));
  }
}



