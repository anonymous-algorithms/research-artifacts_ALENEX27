# Experiment 1 — Performance evaluation

This experiment evaluates the running time and peak memory consumption of main memory algorithm for datasets of increasing size.

## Parameters

The algorithm is run with:

* `k = 35`
* `t` threads, where `t` can be specified by the user
* `p` repetitions for each dataset

The experiments reported in the paper used `t = 20 p=11`.

The command executed for each run is:

```bash
./quotient -i <input_fasta> -o <output_gfa> -k 35 -t <number_of_threads>
```

## Datasets

The experiment uses the following input files:

```text
ecoli50.fa
ecoli100.fa
ecoli200.fa
ecoli400.fa
ecoli800.fa
ecoli1600.fa
```

## Measurements

Each invocation is wrapped with `/usr/bin/time -v`.

For every run, the following measurements are collected:

1. **Total running time** — reported by `/usr/bin/time -v`.
2. **Index construction time** — reported in milliseconds.
3. **Graph construction main step time** — reported in milliseconds.
4. **Peak RSS memory** — `Maximum resident set size` reported by `/usr/bin/time -v`.

For total running time, index construction time, and graph construction time, the **median of the p runs** is reported.

The experiments reported in the paper cleared the filesystem cache after every run. This requires elevated privileges on the host system. Cache clearing is therefore disabled in the artifact.

## Running the experiment manually

The following procedure illustrates how the experiment is executed. Each run is logged separately.

```bash
THREADS=20
REPETITIONS=11
for dataset in ecoli50 ecoli100 ecoli200 ecoli400 ecoli800 ecoli1600; do
    mkdir -p "results/${dataset}"

     for ((run = 1; run <= REPETITIONS; run++)); do
        echo "Dataset: ${dataset}, run: ${run}/${REPETITIONS}"

        /usr/bin/time -v \
            ./quotient \
            -i "data/${dataset}.fa" \
            -o "results/${dataset}/output_${run}.gfa" \
            -k 35 \
            -t "${THREADS}" \
            > "results/${dataset}/run_${run}.log" 2>&1

        # The experiments reported in the paper cleared the filesystem
        # cache between runs. This requires elevated privileges on the host.
        # sync
        # sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
    done
done
```

The complete experiment can also be executed automatically using:

```bash
./workflows/experiment1/run.sh <threads> <repetitions>
```

The automatic script performs the same procedure described above and additionally parses the individual logs and computes the aggregate measurements.

## Output

After completion, the results are available in:

```text
workflows/experiment1/results/
```

The main summary is:

```text
workflows/experiment1/results/summary.csv
```

It contains one row per dataset and the following columns:

```text
dataset,total_time_median,index_construction_median,graph_construction_median,peak_rss_kb
```

The directory also contains the individual logs for all runs.
