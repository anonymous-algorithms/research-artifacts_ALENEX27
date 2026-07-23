#ifndef GFA_WRITER_IMPL_HPP
#define GFA_WRITER_IMPL_HPP

#include "../externals/robin-hood-hashing/src/include/robin_hood.h"
#include <cstdio> // for snprintf
#include <cstdio>
#include <fstream>
#include <string>
#include <type_traits>
#include <vector>
#include<iostream>

template <typename T>
std::string format_P_line(const std::string &name, const std::vector<T> &path) {
  size_t total_size = 2 + name.size() + 1 + 3; // "P\t" + name + "\t" + "\t*\n"
  size_t n = path.size();

  // For each node, max 10 digits + 1 sign + (comma except last)
  // Let's use max 11 per node (10 digits + 1 sign), plus commas for all but
  // last

  int decimals;
  if (sizeof(T) == 4) {
    decimals = 10;
  } else if (sizeof(T) == 8) {
    decimals = 19;
  }

  total_size += n * (decimals + 1) + (n > 0 ? (n - 1) : 0);

  // Select correct format specifiers depending on T
  constexpr bool is_long =
      std::is_same_v<T, long> || std::is_same_v<T, unsigned long>;
  constexpr bool is_long_long =
      std::is_same_v<T, long long> || std::is_same_v<T, unsigned long long>;

  // Format string parts for unsigned abs values:
  const char *fmt_unsigned = nullptr;
  if constexpr (is_long) {
    fmt_unsigned = "%lu";
  } else if constexpr (is_long_long) {
    fmt_unsigned = "%llu";
  } else {
    fmt_unsigned = "%u"; // default to unsigned int
  }

  std::string line;
  line.resize(total_size);

  size_t pos = 0;
  line[pos++] = 'P';
  line[pos++] = '\t';

  // copy name
  memcpy(&line[pos], name.data(), name.size());
  pos += name.size();

  line[pos++] = '\t';

  for (size_t i = 0; i < n; i++) {
    int id = path[i];
    char sign = (id > 0) ? '+' : '-';
    unsigned int abs_id = (id > 0) ? id : -id;

    // convert abs_id to string fast
    char buf[20];
    int len = snprintf(buf, sizeof(buf), fmt_unsigned, abs_id);
    memcpy(&line[pos], buf, len);
    pos += len;

    line[pos++] = sign;

    if (i + 1 < n) {
      line[pos++] = ',';
    }
  }

  line[pos++] = '\t';
  line[pos++] = '*';
  line[pos++] = '\n';

  // resize to actual used size (in case of smaller abs_id lengths)
  line.resize(pos);

  return line;
}

template <typename T>
std::string format_L_lines_for_path(
    const std::vector<T> &path,
    robin_hood::unordered_set<robin_hood::pair<T, T>> &edges) {
  size_t max_lines = path.size() > 1 ? path.size() - 1 : 0;
  size_t max_bytes = max_lines * 64; // generous estimate per line
  std::string buffer;
  buffer.resize(max_bytes);

  constexpr bool is_long =
      std::is_same_v<T, long> || std::is_same_v<T, unsigned long>;
  constexpr bool is_long_long =
      std::is_same_v<T, long long> || std::is_same_v<T, unsigned long long>;

  const char *fmt_unsigned = nullptr;
  if constexpr (is_long) {
    fmt_unsigned = "%lu";
  } else if constexpr (is_long_long) {
    fmt_unsigned = "%llu";
  } else {
    fmt_unsigned = "%u"; // default to unsigned int
  }

  size_t pos = 0;
  for (size_t i = 0; i + 1 < path.size(); ++i) {
    T from = path[i];
    T to = path[i + 1];
    robin_hood::pair<T, T> edge = {path[i], path[i + 1]};
    robin_hood::pair<T, T> rev = {-path[i + 1], -path[i]};
    if ((edges.find(edge) != edges.end()) || (edges.find(rev) != edges.end())) {
      continue;
    }

    edges.insert(edge);

    char sign_left = (from > 0) ? '+' : '-';
    char sign_right = (to > 0) ? '+' : '-';
    auto abs_left = std::abs(from);
    auto abs_right = std::abs(to);

    int written = std::snprintf(&buffer[pos], max_bytes - pos,
                                ("L\t" + std::string(fmt_unsigned) + "\t%c\t" +
                                 std::string(fmt_unsigned) + "\t%c\t0M\n")
                                    .c_str(),
                                abs_left, sign_left, abs_right, sign_right);
    pos += written;
  }

  buffer.resize(pos); // trim unused space
  return buffer;
}

template <typename T1, typename T2>
void write_gfa(const std::vector<std::pair<T1, T1>> &labels,
               const std::vector<std::vector<T2>> &paths,
               const std::string &filename, const std::string &sequence,
               const std::vector<std::string> &names) {
  robin_hood::unordered_set<robin_hood::pair<T2, T2>> edges;
  std::ofstream output;
  output.open(filename);

  // GFA version
  output << "H\tVN:Z:1.0\n";

  // Write S lines
  for (size_t i = 0; i < labels.size(); i++) {
    output << "S\t" << i + 1 << "\t";
		//std::cout<<labels[i].first<<'\t'<<labels[i].second<<'\n';
    if (labels[i].second == 0) {
      output << "*";
    } else {
      output.write(&sequence[labels[i].first], labels[i].second);
    }
    output << "\n";
  }

  // write L lines
  for (const auto &path : paths) {
    std::string batch_L = format_L_lines_for_path(path, edges);
    output.write(batch_L.data(), batch_L.size());
  }

  // Write P lines
  for (size_t i = 0; i < paths.size(); i++) {
    auto line = format_P_line(names[i], paths[i]);
    output.write(line.data(), line.size());
  }
}

#endif // GFA_WRITER_IMPL_HPP
