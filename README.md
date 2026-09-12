# quotient

Anonymous artifact version for peer review.

This repository contains the anonymized software artifact submitted for peer review. The tool is temporarily referred to as `quotient` for the purposes of anonymous evaluation. The final software name may differ from the one used in this repository.

`quotient` is a tool for constructing k-mer quotient graphs for pangenomic applications.

## Installation

The recommended way to run the artifact is using Docker. The provided `Dockerfile` contains the required system and software dependencies, including the C++ build environment, NCBI Datasets CLI, R, and Biopython.

First, clone the repository together with its submodules:

```bash
git clone --recursive <repository_url>
cd <repository_directory>
```

If the repository was cloned without submodules, initialize them with:

```bash
git submodule update --init --recursive
```

Build the Docker image:

```bash
docker build -t quotient-artifact .
```

The Docker image does not contain the experimental datasets or generated results. These are mounted from the host when running the artifact.

## Running all experiments

The complete experimental pipeline is provided by `run.sh`. It builds `quotient`, downloads and prepares the required datasets, and runs all four experiments.

```bash
./run.sh <threads> <repetitions> <run_alfapang>
```

For example:

```bash
./run.sh 20 11 false
```

The arguments are:

* `<threads>` — number of CPU threads used by the experiments.
* `<repetitions>` — number of repetitions for each experiment.
* `<run_alfapang>` — whether the historical AlfaPang baseline should be executed. Must be either `true` or `false`.

The `repetitions` parameter controls how many times each measurement is repeated. The reported running times are computed as the **median over the repetitions**. The experiments reported in the paper used **11 repetitions**.

The Docker image can be run as follows:

```bash
docker run --rm \
    --ulimit nofile=128000:128000 \
    -v "$(pwd)/data:/artifact/data" \
    -v "$(pwd)/results:/artifact/results" \
    quotient-artifact 20 11 false
```

The `data/` directory is used for downloaded and prepared datasets, while `results/` contains the experimental results.

The experiments reported in the paper used:

```text
threads = 20
repetitions = 11
```

Running the complete pipeline with these settings can require substantial computational resources. Even without running AlfaPang, reproducing all experiments may require approximately **250 GB of RAM** and up to **1 TB of storage**, depending on the system and intermediate files.

Running one repetition of each experiment should still be expected to require **several days of computation** on a sufficiently large machine.

## Experiment workflows

The individual experiments are described in detail in the `workflows/` directory.

Each workflow documents:

* the datasets used,
* the parameters used in the paper,
* the commands required to run the experiment manually,
* the measurements collected,
* and the expected output files.

The workflows can be followed independently without using `run.sh`. The corresponding scripts in `scripts/` automate the same procedures.

The available workflows are:

* `data_preparation.md` 
* `workflows/experiment1.md` — main-memory performance evaluation.
* `workflows/experiment2.md` — external-memory performance evaluation.
* `workflows/experiment3.md` — effect of the k-mer size.
* `workflows/experiment4.md` — comparison with main-memory, external-memory, and AlfaPang implementations.

## AlfaPang

Experiment 4 includes a comparison with the historical version of AlfaPang used to obtain the results reported in the paper.

Running AlfaPang from scratch is **extremely resource-intensive**. In particular, the largest dataset can require approximately **2.5 days for a single repetition** and around **358 GB of RAM**. Repeating this experiment 11 times would therefore take on the order of **weeks**.

For this reason, the artifact provides the AlfaPang results used in the paper as a CSV file..

To execute the historical AlfaPang version and reproduce these measurements from scratch, use:

```bash
./run.sh <threads> <repetitions> true
```

This option should only be used on a sufficiently large machine with substantial available memory, storage, and computation time.

The AlfaPang version used for the experiment is the historical version corresponding to the results reported in the paper, rather than the latest version of the software.

## Direct usage

The `quotient` executable can also be used independently of the experimental scripts:

```bash
./quotient -i <input_fasta> -o <output_gfa> -k <kmer_size> -t <number_of_threads> [-e]
```

Arguments:

* `-i <input_fasta>`
  Input FASTA file containing all sequences used for graph construction. All input sequences should be provided in this single file.

* `-o <output_gfa>`
  Output GFA file.

* `-k <kmer_size>`
  Size of the k-mer. The value has to be odd.

* `-t <number_of_threads>`
  Number of CPU threads used during construction.

* `-e`
  Optional flag. If enabled, the index is constructed in external memory using psascan.

Example:

```bash
./quotient -i genomes.fa -o graph.gfa -k 31 -t 16
```

External-memory construction:

```bash
./quotient -i genomes.fa -o graph.gfa -k 31 -t 16 -e
```

## System requirements

The artifact is intended to run on a Linux system with:

* x86-64 CPU with AVX2 and FMA support,
* sufficient RAM and disk space for the selected experiment,
* Docker with support for setting the container's file-descriptor limit.

The experiments may create a large number of temporary files. The Docker command above therefore sets:

```bash
--ulimit nofile=128000:128000
```

If the host imposes a lower hard limit, Docker cannot raise the container limit above that value.

The cache-clearing commands used for the experiments reported in the paper require elevated host privileges and are therefore not executed automatically by the artifact scripts.
