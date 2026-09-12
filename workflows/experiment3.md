# Experiment 3 — Performance across k values

This experiment evaluates the running time of `quotient` for different values of the k-mer size.

## Parameters

The algorithm is run with:

* input dataset: `ecoli100.fa`
* `k` values from 5 to 299, with a step of 2
* `t` threads, where `t` can be specified by the user
* 11 repetitions for each value of `k`

The experiments reported in the paper used `t = 20`.

The command executed for each run is:

```bash
./quotient -i <input_fasta> -o <output_gfa> -k <kmer_size> -t <number_of_threads>
```

## Dataset

The experiment uses:

```text
ecoli100.fa
```

## Measurements

Each invocation is wrapped with `/usr/bin/time -v`.

For every run, the following measurements are collected:

1. **Total running time** — reported by `/usr/bin/time -v`.
2. **Graph construction main step time** — reported in milliseconds.

For both measurements, the **median of the 11 runs** is reported for each value of `k`.

The experiments reported in the paper cleared the filesystem cache after every run. This step is commented out in the artifact because it requires elevated privileges on the host system.

## Running the experiment manually

The following code illustrates the procedure used for the experiment:

```bash
for k in $(seq 5 2 299); do
    mkdir -p "results/k_${k}"

    for run in {1..11}; do
        echo "k=${k}, run=${run}/11"

        /usr/bin/time -v \
            ./quotient \
            -i data/ecoli100.fa \
            -o "results/k_${k}/output.gfa" \
            -k "${k}" \
            -t "${THREADS}" \
            > "results/k_${k}/run_${run}.log" 2>&1

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
./workflows/experiment3/run.sh 20
```

The script performs the procedure above, parses the logs, and computes the median measurements for every value of `k`.

## Output

After completion, the results are available in:

```text
workflows/experiment3/results/
```

The main summary is:

```text
workflows/experiment3/results/summary.csv
```

It contains one row per value of `k` and the following columns:

```text
k,total_time_median_ms,graph_construction_median_ms
```

The directory also contains the individual logs for all runs.
