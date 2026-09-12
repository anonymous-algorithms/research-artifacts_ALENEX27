# Experiment 2 — External-memory evaluation

This experiment evaluates the external-memory behavior of `quotient`, including I/O volume and storage usage.

## Parameters

The algorithm is run with:

* `-e` enabled
* `t` threads, where `t` can be specified by the user
* `p` repetitions for each dataset
* a dataset-specific value of `k`

The experiments reported in the paper used `t = 20, p=11`.

The `k` value is selected according to the dataset:

| Dataset        | `k` |
| -------------- | --: |
| `ecoli400.fa`  | 267 |
| `ecoli800.fa`  | 371 |
| `ecoli1600.fa` | 533 |
| `ecoli3412.fa` | 745 |

The command executed for each run is:

```bash
./quotient -i <input_fasta> -o <output_gfa> -k <kmer_size> -t <number_of_threads> -e
```

## Datasets

The experiment uses the following input files:

```text
ecoli400.fa
ecoli800.fa
ecoli1600.fa
ecoli3412.fa
```

## Measurements

Each invocation is wrapped with `/usr/bin/time -v`.

For every run, the following measurements are collected:

1. **Total running time** — reported by `/usr/bin/time -v`.
2. **Peak RSS memory** — `Maximum resident set size` reported by `/usr/bin/time -v`.
3. **Block-layer read I/O** — `read_bytes` from `/proc/$PID/io`.
4. **Block-layer write I/O** — `write_bytes` from `/proc/$PID/io`.
5. **System-call-level read I/O** — `rchar` from `/proc/$PID/io`.
6. **System-call-level write I/O** — `wchar` from `/proc/$PID/io`.
7. **Storage usage** — total storage occupied by the input dataset and the files produced by a single invocation of `quotient`.

The `read_bytes` and `write_bytes` values measure the actual number of bytes transferred to and from the block layer. The `rchar` and `wchar` values measure bytes read and written at the system-call level, including I/O served from the page cache.

Storage usage is measured **only after the first invocation for each dataset**. It includes the input dataset and all files produced by that invocation. The generated files are then removed before the remaining repetitions.

For total running time, index construction time, graph construction time, and I/O measurements, the **median of the 11 runs** is reported.

The experiments reported in the paper cleared the filesystem cache after every run. This step is commented out in the artifact because it requires elevated privileges on the host system.

## Running the experiment manually

The following code illustrates the procedure used for the experiment. The process ID is used to read `/proc/$PID/io`, and the output files are overwritten between runs.

```bash
declare -A K_VALUES=(
    [ecoli400]=267
    [ecoli800]=371
    [ecoli1600]=533
    [ecoli3412]=745
)

for dataset in ecoli400 ecoli800 ecoli1600 ecoli3412; do
    k="${K_VALUES[$dataset]}"

    mkdir -p "results/${dataset}"

    for run in {1..11}; do
        echo "Dataset: ${dataset}, k=${k}, run=${run}/11"

        /usr/bin/time -v \
            ./quotient \
            -i "data/${dataset}.fa" \
            -o "results/${dataset}/output.gfa" \
            -k "${k}" \
            -t "${THREADS}" \
            -e \
            > "results/${dataset}/run_${run}.log" 2>&1 &

        PID=$!

        # I/O counters are collected from /proc/${PID}/io.
        # The automatic script records the final values after
        # the process terminates.

        wait "${PID}"

        cat "/proc/${PID}/io" > "results/${dataset}/run_${run}.io"

        if [[ "${run}" -eq 1 ]]; then
            # Storage is measured only for the first run.
            # It includes the input dataset and generated files.
            du -sb \
                "data/${dataset}.fa" \
                "results/${dataset}/output.gfa" \
                > "results/${dataset}/storage.txt"
        fi

        # Remove files produced by quotient before the next run.
        rm -f "results/${dataset}/output.gfa"

        # The experiments reported in the paper cleared the filesystem
        # cache between runs. This requires elevated privileges on the host.
        #
        # sync
        # sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
    done
done
```

The experiment can also be executed automatically using the provided script:

```bash
./workflows/experiment2/run.sh <threads> <repetitions>
```

The script performs the procedure above and parses the resulting logs and I/O measurements to compute the aggregate results.

## Output

After completion, the results are available in:

```text
workflows/experiment2/results/
```

The main summary is:

```text
workflows/experiment2/results/summary.csv
```

It contains one row per dataset and the following columns:

```text
dataset,total_time_,peak_rss,read_bytes_median,write_bytes_median,rchar_median,wchar_median,storage_bytes
```

The directory also contains the individual logs and raw I/O measurements for all runs.


The resulting `summary.csv` contains the aggregated measurements used to generate the corresponding results in the paper.
