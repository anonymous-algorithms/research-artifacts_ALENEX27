#include "sequence_utils.h"
#include <array>
#include <immintrin.h>
#include <iostream>
#include <omp.h>
void translate_sequence(std::string &sequence) {
  const std::unordered_map<char, char> translation_table{
      {'H', 'N'}, {'D', 'N'}, {'K', 'N'}, {'M', 'N'}, {'R', 'N'},
      {'S', 'N'}, {'W', 'N'}, {'Y', 'N'}, {'V', 'N'}, {'B', 'N'},
      {'a', 'A'}, {'c', 'C'}, {'t', 'T'}, {'g', 'G'}, {'n', 'N'},
      {'h', 'N'}, {'d', 'N'}, {'k', 'N'}, {'m', 'N'}, {'r', 'N'},
      {'s', 'N'}, {'w', 'N'}, {'y', 'N'}, {'v', 'N'}, {'b', 'N'}};
  for (char &c : sequence) {
    auto it = translation_table.find(c);
    if (it != translation_table.end()) {
      c = it->second;
    }
  }
}

std::string get_reversed_strand(const std::string_view seq) {
  static char complement[256] = {0}; // Static array initialized only once

  if (complement['A'] == 0) {
    complement['A'] = 'T';
    complement['T'] = 'A';
    complement['C'] = 'G';
    complement['G'] = 'C';
    complement['N'] = 'N';
    complement['$'] = '$';
  }

  size_t n = seq.size();
  std::string rev_comp(n, ' ');
  size_t i = 0;
  for (; i < n; i++) {
    rev_comp[n - 1 - i] = complement[(unsigned char)seq[i]];
  }
  return rev_comp;
}

std::string get_reversed_strand(const char *ptr, size_t k) {
  static char complement[256] = {0}; // Static array initialized only once

  if (complement['A'] == 0) {
    complement['A'] = 'T';
    complement['T'] = 'A';
    complement['C'] = 'G';
    complement['G'] = 'C';
    complement['N'] = 'N';
    complement['$'] = '$';
  }

  std::string rev_comp(k, ' ');
  size_t i = 0;
  for (; i < k; i++) {
    rev_comp[i] = complement[(unsigned char)ptr[k - 1 - i]];
  }
  return rev_comp;
}

 uint8_t base_hash(std::string s) {
  static constexpr std::array<uint8_t, 256> lookup = [] {
    std::array<uint8_t, 256> table = {};
    table['A'] = 1;
    table['C'] = 2;
    table['T'] = 3;
    table['G'] = 4;
    table['N'] = 5;
    table['$'] = 6;
    table['X'] = 7;
    return table;
  }();

  uint8_t i = lookup[s[0]];
  uint8_t j = lookup[s[1]];

  return i << 3 | j;
}



void print_avx(__m256i vec, const char* label) {
    uint8_t tmp[32];
    _mm256_storeu_si256((__m256i*)tmp, vec);
    printf("%s: ", label);
    for (int i = 0; i < 32; ++i)
        printf("%02x ", tmp[i]);
    printf("\n");
}

std::string get_reversed_strand_avx2(const std::string_view input) {

  static char scalar_complement[256] = {
      0}; // Static array initialized only once

  if (scalar_complement['A'] == 0) {
    scalar_complement['A'] = 'T';
    scalar_complement['T'] = 'A';
    scalar_complement['C'] = 'G';
    scalar_complement['G'] = 'C';
    scalar_complement['N'] = 'N';
    scalar_complement['$'] = '$';
  }

  size_t len = input.size();
  std::string output(len, '\0');

  //alignas(32) static const uint8_t complement_lut[32] = {
  //    'T', // 0: A
  //    'G', // 1: C
  //    'A', // 2: T
  //    'C', // 3: G
  //    'N', // 4: N
  //    '$', // 5: $
  //    0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //    0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
alignas(32) static const uint8_t complement_lut[32] = {
    'T', 'G', 'A', 'C', 'N', '$',  // valid mapping for indices 0–5
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', // fill remaining bytes in lower lane with a safe default (or you can repeat, but you only need indices 0–5)
    'T', 'G', 'A', 'C', 'N', '$',  // duplicate the first 6 bytes in the upper lane
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N'         // fill remaining bytes in upper lane
};

  // Step 1: reverse bytes inside each 128-bit lane
  const __m256i lane_reverse_mask =
      _mm256_setr_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15,
                       14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

  // static const __m256i reverse_mask = _mm256_setr_epi8(
  //   31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14,
  // 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
  static const __m256i lut = _mm256_load_si256((const __m256i *)complement_lut);

  size_t i = 0;
  size_t full_blocks = len / 32;

  for (size_t b = 0; b < full_blocks; ++b) {
    const uint8_t *src = (const uint8_t *)input.data() + len - 32 * (b + 1);
    uint8_t *dst = (uint8_t *)output.data() + i;

    __m256i in = _mm256_loadu_si256((const __m256i *)src);

//print_avx(in, "Input");
    // Reverse bytes in lanes
    __m256i rev_lanes = _mm256_shuffle_epi8(in, lane_reverse_mask);

    // Step 2: swap lanes to complete full 32-byte reversal
    //__m256i rev = _mm256_permute2x128_si256(rev_lanes, rev_lanes, 1);
    __m256i rev = _mm256_permute2x128_si256(rev_lanes, rev_lanes, 0x01);
 //print_avx(rev, "reversed");
    //__m256i rev = _mm256_shuffle_epi8(in, reverse_mask);

    // Compare with nucleotides to generate mask vectors
    __m256i eq_a = _mm256_cmpeq_epi8(rev, _mm256_set1_epi8('A'));
    __m256i eq_c = _mm256_cmpeq_epi8(rev, _mm256_set1_epi8('C'));
    __m256i eq_t = _mm256_cmpeq_epi8(rev, _mm256_set1_epi8('T'));
    __m256i eq_g = _mm256_cmpeq_epi8(rev, _mm256_set1_epi8('G'));
    __m256i eq_n = _mm256_cmpeq_epi8(rev, _mm256_set1_epi8('N'));
    __m256i eq_d = _mm256_cmpeq_epi8(rev, _mm256_set1_epi8('$'));

    // idx starts with all zeros (index 0 = 'T' complement of 'A')
    __m256i idx = _mm256_setzero_si256();
    // Set idx byte to 1 if eq_c, 2 if eq_t, 3 if eq_g, 4 if eq_n, 5 if eq_d
    // Use bitwise OR and masking to selectively set bytes

    __m256i one = _mm256_set1_epi8(1);
    __m256i two = _mm256_set1_epi8(2);
    __m256i three = _mm256_set1_epi8(3);
    __m256i four = _mm256_set1_epi8(4);
    __m256i five = _mm256_set1_epi8(5);

    idx = _mm256_or_si256(idx, _mm256_and_si256(eq_c, one));
    idx = _mm256_or_si256(idx, _mm256_and_si256(eq_t, two));
    idx = _mm256_or_si256(idx, _mm256_and_si256(eq_g, three));
    idx = _mm256_or_si256(idx, _mm256_and_si256(eq_n, four));
    idx = _mm256_or_si256(idx, _mm256_and_si256(eq_d, five));

    __m256i out_vec = _mm256_shuffle_epi8(lut, idx);
 //print_avx(out_vec, "out_vec");
    _mm256_storeu_si256((__m256i *)dst, out_vec);
    i += 32;
  }
  // Scalar fallback for remaining bytes
  for (; i < len; i++) {
    output[i] = scalar_complement[input[len - i - 1]];
  }
//  std::cout << "Input: " << input << "\n";
//  std::cout << "Result: " << output << "\n";

  return output;
}


void rc_inplace(std::string &T, const int threads) {
    const size_t n = T.size();
    T.resize(2 * n + 1);
    T[n] = '$';

    char *dst = T.data() + n + 1;
    const char *src = T.data();

    static char scalar_complement[256] = {0};
    if (scalar_complement['A'] == 0) {
        scalar_complement['A'] = 'T';
        scalar_complement['T'] = 'A';
        scalar_complement['C'] = 'G';
        scalar_complement['G'] = 'C';
        scalar_complement['N'] = 'N';
        scalar_complement['$'] = '$';
    }

    alignas(32) static const uint8_t complement_lut[32] = {
        'T', 'G', 'A', 'C', 'N', '$', 
        'N','N','N','N','N','N','N','N','N','N',
        'T', 'G', 'A', 'C', 'N', '$',
        'N','N','N','N','N','N','N','N','N','N'
    };

    const __m256i lane_reverse_mask =
        _mm256_setr_epi8(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                         15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    static const __m256i lut = _mm256_load_si256((const __m256i*)complement_lut);

    const size_t full_blocks = n / 32;
    size_t i = 0;
#pragma omp parallel for schedule(static) num_threads(threads)
    for (size_t b = 0; b < full_blocks; ++b) {
        const uint8_t *s = (const uint8_t*)src + n - 32 * (b + 1);
        uint8_t *d = (uint8_t*)dst + (b)*32;//+i

        __m256i in = _mm256_loadu_si256((const __m256i*)s);
        __m256i rev_lanes = _mm256_shuffle_epi8(in, lane_reverse_mask);
        __m256i rev = _mm256_permute2x128_si256(rev_lanes, rev_lanes, 0x01);

        __m256i eq_a = _mm256_cmpeq_epi8(rev, _mm256_set1_epi8('A'));
        __m256i eq_c = _mm256_cmpeq_epi8(rev, _mm256_set1_epi8('C'));
        __m256i eq_t = _mm256_cmpeq_epi8(rev, _mm256_set1_epi8('T'));
        __m256i eq_g = _mm256_cmpeq_epi8(rev, _mm256_set1_epi8('G'));
        __m256i eq_n = _mm256_cmpeq_epi8(rev, _mm256_set1_epi8('N'));
        __m256i eq_d = _mm256_cmpeq_epi8(rev, _mm256_set1_epi8('$'));

        __m256i idx = _mm256_setzero_si256();
        __m256i one = _mm256_set1_epi8(1);
        __m256i two = _mm256_set1_epi8(2);
        __m256i three = _mm256_set1_epi8(3);
        __m256i four = _mm256_set1_epi8(4);
        __m256i five = _mm256_set1_epi8(5);

        idx = _mm256_or_si256(idx, _mm256_and_si256(eq_c, one));
        idx = _mm256_or_si256(idx, _mm256_and_si256(eq_t, two));
        idx = _mm256_or_si256(idx, _mm256_and_si256(eq_g, three));
        idx = _mm256_or_si256(idx, _mm256_and_si256(eq_n, four));
        idx = _mm256_or_si256(idx, _mm256_and_si256(eq_d, five));

        __m256i out_vec = _mm256_shuffle_epi8(lut, idx);
        _mm256_storeu_si256((__m256i*)d, out_vec);

        i += 32;
    }

    // pozostałe znaki (gdy n % 32 != 0)
    for (; i < n; ++i)
        dst[i] = scalar_complement[(unsigned char)src[n - 1 - i]];
}

