#include "Indexing.h"
#include "libsais.h"
#include "libsais64.h"
void build_phi(const std::string &T, std::vector<int32_t> &phi,
               const int threads ) {
  std::vector<int32_t> SA;
  size_t n = T.size();
  std::cout << "Building SA..." << std::endl;

  // --- 32-bit version ---
  SA.resize(n);
  if (libsais_omp(reinterpret_cast<const unsigned char *>(T.c_str()), SA.data(),
                  static_cast<int>(n), 0, nullptr, threads) != 0) {
    throw std::runtime_error("libsais failed");
  }

  std::cout << "Building phi..." << std::endl;
  phi.resize(n);
  phi[SA[0]] = SA[n - 1];
#pragma omp parallel for schedule(static) num_threads(threads)
  for (int i = 1; i < static_cast<int>(n); i++) {
    phi[SA[i]] = SA[i - 1];
  }
}

void build_phi(const std::string &T, std::vector<int64_t> &phi,
               const int threads ) {
  std::vector<int64_t> SA;
  size_t n = T.size();
  SA.resize(n);
  // --- 64-bit version ---
  std::cout << "Building SA...\n";
  if (libsais64_omp(reinterpret_cast<const unsigned char *>(T.c_str()),
                    SA.data(), n, 0, nullptr, threads) != 0) {
    throw std::runtime_error("libsais64 failed");
  }
  std::cout << "Building phi (64-bit)..." << std::endl;
  phi.resize(n);
  phi[SA[0]] = SA[n - 1]; // było -1
#pragma omp parallel for schedule(static) num_threads(threads)
  for (size_t i = 1; i < n; i++)
    phi[SA[i]] = SA[i - 1];
}


