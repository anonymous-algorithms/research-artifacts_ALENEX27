# Experiment 4 — Comparison with AlfaPang

This experiment compares the running time and peak memory usage of `quotient` in main-memory and external-memory modes against the historical AlfaPang implementation used in the previous work.

## Parameters

* `t` — number of threads, specified by the user; the paper used `t=20`.
* `k`:

  * `ecoli50.fa`: `47`
  * `ecoli100.fa`: `165`
  * `ecoli200.fa`: `217`
  * `ecoli400.fa`: `267`
  * `ecoli800.fa`: `371`
  * `ecoli1600.fa`: `533`
  * `ecoli3412.fa`: `745`
* `p` repetitions for each configuration.

The `quotient` implementation is evaluated in two modes:

* **main-memory mode:** without the `-e` flag;
* **external-memory mode:** with the `-e` flag.

The main-memory version is not expected to complete on the largest dataset (`ecoli3412.fa`), so this configuration is omitted for that dataset.

For comparison with the previous work, **AlfaPang is compiled from the exact commit used for the reported historical results**, rather than from its current/latest version.

## Datasets

All datasets prepared in `data_preparation.md` are used:

```text
ecoli50.fa
ecoli100.fa
ecoli200.fa
ecoli400.fa
ecoli800.fa
ecoli1600.fa
ecoli3412.fa
```

The historical AlfaPang results used in the paper are provided as:

```text
data/alfapang.csv
```

## Commands

For `quotient` in main-memory mode:

```bash
./quotient -i <input_fasta> -o <output_gfa> -k <kmer_size> -t <number_of_threads>
```

For `quotient` in external-memory mode:

```bash
./quotient -i <input_fasta> -o <output_gfa> -k <kmer_size> -t <number_of_threads> -e
```

For AlfaPang:

```bash
./AlfaPang <input.fa> <output.gfa> <k>
```


## Measurements

For every configuration, the following quantities are measured:

1. **Wall clock time** — obtained using `/usr/bin/time -v`.
2. **Peak memory usage** — `Maximum resident set size` reported by `/usr/bin/time -v`.


The generated graph is written to a single `output.gfa` and overwritten between runs. Logs are retained.

The filesystem cache clearing used in the paper is left commented out because it requires elevated privileges on the host:

```bash
# sync
# sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
```

## Running the experiment manually

For each dataset, run `quotient` in both modes and AlfaPang using the corresponding `k` value. Each configuration is repeated 11 times.

```bash
declare -A K_VALUES=(
    [ecoli50]=47
    [ecoli100]=165
    [ecoli200]=217
    [ecoli400]=267
    [ecoli800]=371
    [ecoli1600]=533
    [ecoli3412]=745
)

for dataset in "${!K_VALUES[@]}"; do
    k="${K_VALUES[$dataset]}"

    ./quotient -i "data/${dataset}.fa" -o "results/output.gfa" \
        -k "$k" -t "$THREADS"

    ./quotient -i "data/${dataset}.fa" -o "results/output.gfa" \
        -k "$k" -t "$THREADS" -e

    ./AlfaPang "data/${dataset}.fa" "results/output.gfa" "$k"
done
```

The main-memory `quotient` configuration is omitted for `ecoli3412.fa`.

The experiment can also be executed automatically using the provided script:

```bash
./workflows/experiment4/run.sh <threads> <repetitions>
```

By default, the script uses the precomputed AlfaPang results from `data/alfapang.csv` and does not rerun AlfaPang.

## AlfaPang runtime

Running AlfaPang from scratch can be very time-consuming and memory-intensive. In our experimental setup, a **single repetition** on the largest dataset (`ecoli3412.fa`) takes approximately **2.5 days** and uses approximately **358 GB of RAM**. Since each configuration is repeated 11 times, reproducing the complete AlfaPang experiment on this dataset alone may take approximately **27.5 days**.

Therefore, the artifact includes the AlfaPang results used in the paper in `data/alfapang.csv`. These precomputed results can be used to reproduce the reported comparison without rerunning AlfaPang.

If the evaluator wants to reproduce the AlfaPang measurements from scratch, AlfaPang can be compiled from the specified historical commit and the complete set of runs can be enabled explicitly:

```bash
./workflows/experiment4/run.sh <threads> <repetitions> --run-alfapang
```

This option compiles AlfaPang and executes all AlfaPang repetitions. Due to the substantial time and memory requirements, rerunning AlfaPang is optional and is **not required to reproduce the reported comparison**.

## Output

The script generates a CSV containing one row per dataset and algorithm/configuration:

```text
dataset,algorithm,mode,k,total_time_median,peak_rss
ecoli50,quotient,main-memory,47,...
ecoli50,quotient,external-memory,47,...
ecoli50,AlfaPang,historical,47,...
...
```




The first command is the recommended way to reproduce the complete comparison within a reasonable runtime. The second command is provided for evaluators who want to independently regenerate the AlfaPang measurements and have sufficient computational resources.
