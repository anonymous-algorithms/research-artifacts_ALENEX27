#include "quotient_external.h"


std::vector<int32_t> quotient_external_32(std::string &seq, int k,std::string input_name,  int threads ) {

  using clock = std::chrono::high_resolution_clock;
  auto t1 = clock::now();
  auto n = seq.size();
  std::cout << "Building reverse complement...\n";
  rc_inplace(seq, threads);
  seq.push_back('\0');
  std::string mem = std::to_string(n * 4);
  std::cout << "RAM allowed for Phi computation: " << mem << " bytes\n";
	std::string input_file = input_name+".seq";
  {
		std::ofstream out(input_file);
    out << seq;
  }
	std::string cmd1 = "~/psascan/construct_sa -m " + mem + " " + input_file;
  if (system(cmd1.c_str()) != 0) {
		std::cerr << "psascan failed\n";
    exit(1);
  }
	std::string sa_file = input_file + ".sa5";
	std::string sample_file = input_name+".phi5";
  build_samples(sa_file, sample_file, seq.data(), seq.length(), seq.size(),
                 seq.size());
  std::string().swap(seq);

  DSU<int32_t> UF;
  UF.init(int32_t(n));
  auto t2 = clock::now();
  auto dt1 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  std::cout << "Index construction time: " << dt1.count() << "ms\n";
  std::cout << "Constructing quotient graph...\n";
  t1 = clock::now();
  union_kmers_streamed(sample_file.data(), k, UF, n, seq.data());
  t2 = clock::now();
  auto dt2 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  std::cout << "Graph construction main step time: " << dt2.count() << "ms\n";
  seq.resize(n);   // skróć do oryginalnego rozmiaru
  std::string tmp; // pusty string (mały bufor)
  seq.swap(tmp); // zamień je miejscami — T staje się pusty, bufor zwolniony
  seq = std::move(tmp); // T teraz pusty, tmp idzie do destruktora
  std::cout << "Enumerating and orienting nodes...\n";
  // auto cls = compute_class_ids_vector<T1>(UF, seq, n);
  for (int i = 0; i < UF.parent.size(); i++) {
    UF.parent[i] = UF.find(i);
  }
  std::vector<int32_t> cls = std::move(UF.parent);
  UF.rank.clear();
  UF.rank.shrink_to_fit();
  std::ifstream file(input_file, std::ios::binary);

  if (!file) {
    throw std::runtime_error("File not found "+input_file);
  }

  seq.resize(n);

  file.read(seq.data(), static_cast<std::streamsize>(n));

  if (file.gcount() != static_cast<std::streamsize>(n)) {
    throw std::runtime_error("Plik jest krótszy niż oczekiwano");
  }

  for (int i = 1; i < cls.size(); i++) {
    if (seq[i] == '$') {
      cls[i] = 0;
    }
  }
	std::vector<int32_t> roots;
  for (int i = 1; i < n; i++) {
    if (cls[i] == i) {
      roots.push_back(i);
    }
  }

  for (int id = 0; id < roots.size(); id++) {
    cls[roots[id]] = id + 1;
  }

  size_t ridx = 0;

  for (int i = 1; i < n; i++) {

    // jeśli jesteśmy na roocie → pomijamy
    if (ridx < roots.size() && i == roots[ridx]) {
      ridx++;
      continue;
    }

    int root = cls[i]; // stary pointer do roota

    cls[i] = cls[root] - 2 * (seq[i] != seq[root]) * cls[root];
  }

  std::cout << roots.size() << "\n"; // liczba klas
  return cls;
}

signed_uint40_vector quotient_external_40(std::string &seq, int k, size_t &v,std::string input_name, int threads) {
  using clock = std::chrono::high_resolution_clock;
  auto t1 = clock::now();
  auto n = seq.size();
  // Dodajemy reverse complement
  std::cout << "Building reverse complement...\n";
  rc_inplace(seq, threads);
  seq.push_back('\0');
  std::string mem = std::to_string(n * 4);
  std::cout << "RAM allowed for Phi computation: " << mem << " bytes\n";
  // run_pipeline2(seq, mem);
	std::string input_file = input_name+".seq";
  {
		std::ofstream out(input_file);
    out << seq;
  }
	std::string cmd1 = "~/psascan/construct_sa -m " + mem + " " + input_file;
  if (system(cmd1.c_str()) != 0) {
		std::cerr << "psascan failed\n";
    exit(1);
  }
	std::string sa_file = input_file + ".sa5";
	std::string sample_file = input_name+".phi5";
  build_samples(sa_file, sample_file, seq.data(), seq.length(), seq.size(),
                 seq.size());
  std::string().swap(seq);
  DSU<uint40> UF;
  UF.init(n);
  auto t2 = clock::now();
  auto dt1 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  std::cout << "Index construction time: " << dt1.count() << "ms\n";
  std::cout << "Constructing canonical graph...\n";
  t1 = clock::now();
  union_kmers_streamed(sample_file.data(), k, UF, n, seq.data());
  t2 = clock::now();
  auto dt2 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  std::cout << "Graph construction main step time: " << dt2.count() << "ms\n";

  std::cout << "Enumerating and orienting nodes...\n";

  for (size_t i = 0; i < UF.parent.size(); i++) {
    UF.parent[i] = UF.find(i);
  }
  std::vector<uint40> cls = std::move(UF.parent);
  UF.rank.clear();
  UF.rank.shrink_to_fit();

  std::ifstream file(input_file, std::ios::binary);

  if (!file) {
    throw std::runtime_error("Nie można otworzyć pliku sequence.bin");
  }

  seq.resize(n);

  file.read(seq.data(), static_cast<std::streamsize>(n));

  if (file.gcount() != static_cast<std::streamsize>(n)) {
    throw std::runtime_error("Plik jest krótszy niż oczekiwano");
  }

  for (size_t i = 1; i < cls.size(); i++) {
    if (seq[i] == '$') {
      cls[i] = uint40(0);
    }
  }
	std::vector<uint40> roots;
  for (size_t i = 1; i < n; i++) {
    if (cls[i] == uint40(i)) {
      roots.push_back(i);
    }
  }
  std::vector<uint64_t> bitvector((n + 63) >> 6, ~0ULL);
  for (size_t id = 0; id < roots.size(); id++) {
    cls[roots[id]] = uint40(id + 1);
  }

  size_t ridx = 0;

  for (size_t i = 1; i < n; i++) {

    // jeśli jesteśmy na roocie → pomijamy
    if (ridx < roots.size() && i == roots[ridx]) {
      ridx++;
      continue;
    }

    size_t root = cls[i]; // stary pointer do roota
    cls[i] = cls[root];
    if (seq[i] != seq[root]) {
      bitvector[i >> 6] &= ~(1ULL << (i & 63));
    };
  }
  auto results = signed_uint40_vector(std::move(cls), std::move(bitvector));
  v = roots.size();
  return results;
}

