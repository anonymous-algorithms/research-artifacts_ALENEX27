#!/usr/bin/env bash

set -euo pipefail

# Usage:
#   ./scripts/experiment1.sh <threads> <repetitions>
#
# Example:
#   ./scripts/experiment1.sh 20 11

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <threads> <repetitions>"
    exit 1
fi

THREADS="$1"
REPETITIONS="$2"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

QUOTIENT="${ROOT_DIR}/build/quotient"
DATA_DIR="${ROOT_DIR}/data"
RESULTS_DIR="${ROOT_DIR}/results/experiment1"

K=35

DATASETS=(
    ecoli50
    ecoli100
    ecoli200
    ecoli400
    ecoli800
    ecoli1600
)

if [[ ! -x "${QUOTIENT}" ]]; then
    echo "Error: quotient executable not found:"
    echo "  ${QUOTIENT}"
    exit 1
fi

mkdir -p "${RESULTS_DIR}"

# ---------------------------------------------------------------------------
# Run experiments
# ---------------------------------------------------------------------------

echo "Experiment 1"
echo "Threads:     ${THREADS}"
echo "Repetitions: ${REPETITIONS}"
echo "k:           ${K}"

for dataset in "${DATASETS[@]}"; do
    DATASET_DIR="${RESULTS_DIR}/${dataset}"
    mkdir -p "${DATASET_DIR}"

    echo "Dataset: ${dataset}"

    for ((run = 1; run <= REPETITIONS; run++)); do
        LOG="${DATASET_DIR}/run_${run}.log"
        OUTPUT="${DATASET_DIR}/output.gfa"

        echo "  Run ${run}/${REPETITIONS}"

        /usr/bin/time -v \
            "${QUOTIENT}" \
            -i "${DATA_DIR}/${dataset}.fa" \
            -o "${OUTPUT}" \
            -k "${K}" \
            -t "${THREADS}" \
            > "${LOG}" 2>&1

        # The experiments reported in the paper cleared the filesystem
        # cache between runs. This requires elevated privileges on the host.
        #
        # sync
        # sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
    done

done

# ---------------------------------------------------------------------------
# Generate summary.csv
# ---------------------------------------------------------------------------

SUMMARY="${RESULTS_DIR}/summary.csv"

echo "dataset,Wall clock time (mm:ss),RSS (GB),main loop (s),indexing (s)" \
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

for dataset in "${DATASETS[@]}"; do
    DATASET_DIR="${RESULTS_DIR}/${dataset}"

    WALL_TIMES=()
    RSS_VALUES=()
    MAIN_LOOP_VALUES=()
    INDEXING_VALUES=()

    for ((run = 1; run <= REPETITIONS; run++)); do
        LOG="${DATASET_DIR}/run_${run}.log"

        # GNU time:
        # Elapsed (wall clock) time (h:mm:ss or m:ss): ...
        WALL_TIME="$(
            grep 'Elapsed (wall clock) time' "${LOG}" |
            sed 's/.*: //' |
            tail -n 1
        )"

        WALL_SECONDS="$(
            awk -F: '
                {
                    if (NF == 2)
                        printf "%.6f\n", $1 * 60 + $2
                    else
                        printf "%.6f\n", $1 * 3600 + $2 * 60 + $3
                }
            ' <<< "${WALL_TIME}"
        )"

        RSS_KB="$(
            grep 'Maximum resident set size (kbytes)' "${LOG}" |
            awk '{print $NF}' |
            tail -n 1
        )"

        RSS_GB="$(
            awk -v rss="${RSS_KB}" \
                'BEGIN { printf "%.6f\n", rss / 1024 / 1024 }'
        )"

        # quotient output:
        #
        # Index construction time: ... ms
        # Graph construction main step time: ... ms

     INDEXING_MS="$(
    grep 'Index construction time:' "${LOG}" |
    awk '{value=$NF; sub(/ms$/, "", value); print value}'
)"

MAIN_LOOP_MS="$(
    grep 'Graph construction main step time:' "${LOG}" |
    awk '{value=$NF; sub(/ms$/, "", value); print value}'
)"

        INDEXING_SECONDS="$(
            awk -v ms="${INDEXING_MS}" \
                'BEGIN { printf "%.6f\n", ms / 1000 }'
        )"

        MAIN_LOOP_SECONDS="$(
            awk -v ms="${MAIN_LOOP_MS}" \
                'BEGIN { printf "%.6f\n", ms / 1000 }'
        )"

        WALL_TIMES+=("${WALL_SECONDS}")
        RSS_VALUES+=("${RSS_GB}")
        MAIN_LOOP_VALUES+=("${MAIN_LOOP_SECONDS}")
        INDEXING_VALUES+=("${INDEXING_SECONDS}")
    done

    WALL_MEDIAN="$(
        printf '%s\n' "${WALL_TIMES[@]}" |
        median
    )"

    RSS_MEDIAN="$(
        printf '%s\n' "${RSS_VALUES[@]}" |
        median
    )"

    MAIN_LOOP_MEDIAN="$(
        printf '%s\n' "${MAIN_LOOP_VALUES[@]}" |
        median
    )"

    INDEXING_MEDIAN="$(
        printf '%s\n' "${INDEXING_VALUES[@]}" |
        median
    )"

    # Convert wall-clock seconds to mm:ss.
    WALL_MMSS="$(
        awk -v seconds="${WALL_MEDIAN}" '
            BEGIN {
                minutes = int(seconds / 60)
                remaining = seconds - minutes * 60
                printf "%02d:%05.2f", minutes, remaining
            }
        '
    )"

    echo "${dataset},${WALL_MMSS},${RSS_MEDIAN},${MAIN_LOOP_MEDIAN},${INDEXING_MEDIAN}" \
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
