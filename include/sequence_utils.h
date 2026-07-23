#ifndef SEQUENCE_UTILS_HPP
#define SEQUENCE_UTILS_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <immintrin.h>
#include <cstdint>
void translate_sequence(std::string &sequence);
std::string get_reversed_strand(const std::string_view seq);
std::string get_reversed_strand_avx2(const std::string_view input);
uint8_t base_hash(std::string s);
void rc_inplace(std::string &T, const int threads);
#endif // SEQUENCE_UTILS_HPP

