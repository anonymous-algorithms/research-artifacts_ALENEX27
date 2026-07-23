# quotient

Anonymous artifact version for peer review.

This repository contains the anonymized version of the software artifact submitted for peer review. The tool is temporarily referred to as `quotient` for the purposes of anonymous evaluation. The final software name may differ from the one used in this repository.

`quotient` is a tool for constructing k-mer quotient graphs for pangenomic applications.

## Installation

Clone the repository with its submodules:
```bash
git clone --recursive <repository_url>
cd <repository_directory>
```

If the repository was cloned without submodules, initialize them with:

```bash
git submodule update --init --recursive
```

Build the project:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```
The executable `quotient` will be generated in the build directory.

## Requirements

The following dependencies are required:

- C++ compiler with AVX2 support
- OpenMP
- libdivsufsort
- psascan (included as a git submodule and used for external-memory construction)

## Usage
```bash
./quotient -i <input_fasta> -o <output_gfa> -k <kmer_size> -t <number_of_threads> [-e]
```
Arguments:

- `-i <input_fasta>`  
  Input FASTA file containing all sequences used for graph construction. All input sequences should be provided in this single file.

- `-o <output_gfa>`  
  Output GFA file.

- `-k <kmer_size>`  
  Size of k-mer. The value has to be odd.

- `-t <number_of_threads>`  
  Number of CPU threads used during construction.

- `-e`  
  Optional flag. If enabled, the index is constructed in external memory using psascan.

Example:
```bash
./quotient -i genomes.fa -o graph.gfa -k 31 -t 16
```
External-memory construction:
```bash
./quotient -i genomes.fa -o graph.gfa -k 31 -t 16 -e
```
