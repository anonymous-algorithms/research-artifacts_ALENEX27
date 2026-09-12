#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <threads> <repetitions>"
    exit 1
fi

THREADS="$1"
REPETITIONS="$2"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

QUOTIENT="${ROOT_DIR}/build/quotient"
INPUT="${ROOT_DIR}/data/ecoli100.fa"
RESULTS_DIR="${ROOT_DIR}/results/experiment3"

K_VALUES=( $(seq 5 2 299) )

if [[ ! -x "${QUOTIENT}" ]]; then
    echo "Error: quotient executable not found:"
    echo "  ${QUOTIENT}"
    exit 1
fi

if [[ ! -f "${INPUT}" ]]; then
    echo "Error: input dataset not found:"
    echo "  ${INPUT}"
    exit 1
fi

mkdir -p "${RESULTS_DIR}"

echo "Experiment 3"
echo "Threads:     ${THREADS}"
echo "Repetitions: ${REPETITIONS}"
echo "Dataset:     ecoli100.fa"
echo "k values:    5..299 (step 2)"
echo

for k in "${K_VALUES[@]}"; do
    DATASET_DIR="${RESULTS_DIR}/k_${k}"
    mkdir -p "${DATASET_DIR}"

    echo "k=${k}"

    for ((run = 1; run <= REPETITIONS; run++)); do
        LOG="${DATASET_DIR}/run_${run}.log"
        OUTPUT="${DATASET_DIR}/output.gfa"

        echo "  Run ${run}/${REPETITIONS}"

        /usr/bin/time -v \
            "${QUOTIENT}" \
            -i "${INPUT}" \
            -o "${OUTPUT}" \
            -k "${k}" \
            -t "${THREADS}" \
            > "${LOG}" 2>&1

        # The experiments reported in the paper cleared the filesystem
        # cache between runs. This requires elevated privileges on the host.
        #
        # sync
        # sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
    done

    echo
done

SUMMARY="${RESULTS_DIR}/summary.csv"

echo "k,total_time_median_s,graph_construction_median_ms" \
    > "${SUMMARY}"

median() {
    sort -n |
        awk '
            {
                values[NR] = $1
            }
            END {
                if (NR % 2 == 1)
                    printf "%.6f\n", values[(NR + 1) / 2]
                else
                    printf "%.6f\n", (values[NR / 2] + values[NR / 2 + 1]) / 2
            }
        '
}

for k in "${K_VALUES[@]}"; do
    DATASET_DIR="${RESULTS_DIR}/k_${k}"

    TOTAL_TIMES=()
    GRAPH_VALUES=()

    for ((run = 1; run <= REPETITIONS; run++)); do
        LOG="${DATASET_DIR}/run_${run}.log"

        WALL_TIME="$(
            grep 'Elapsed (wall clock) time' "${LOG}" |
            sed 's/.*: //' |
            tail -n 1
        )"

        WALL_TIME_S="$(
            awk -F: '
                {
                    if (NF == 2)
                        printf "%.6f\n", ($1 * 60 + $2) 
                    else
                        printf "%.6f\n", ($1 * 3600 + $2 * 60 + $3) 
                }
            ' <<< "${WALL_TIME}"
        )"

        GRAPH_MS="$(
    grep 'Graph construction main step time:' "${LOG}" |
    awk '{value=$NF; sub(/ms$/, "", value); print value}'
)"
echo "${GRAPH_MS}"

        TOTAL_TIMES+=("${WALL_TIME_S}")
        GRAPH_VALUES+=("${GRAPH_MS}")
    done

    TOTAL_TIME_MEDIAN="$(
        printf '%s\n' "${TOTAL_TIMES[@]}" |
        median
    )"

    GRAPH_MEDIAN="$(
        printf '%s\n' "${GRAPH_VALUES[@]}" |
        median
    )"

    echo "${k},${TOTAL_TIME_MEDIAN},${GRAPH_MEDIAN}" \
        >> "${SUMMARY}"
done

echo
echo "Experiment completed."
echo
echo "Summary:"
cat "${SUMMARY}"
echo
echo "Results stored in:"
echo "  ${RESULTS_DIR}"
