#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 2 || $# -gt 3 ]]; then
    echo "Usage: $0 <threads> <repetitions> [--run-alfapang]"
    exit 1
fi

THREADS="$1"
REPETITIONS="$2"
RUN_ALFAPANG=false

if [[ $# -eq 3 ]]; then
    if [[ "$3" != "--run-alfapang" ]]; then
        echo "Error: unknown option: $3"
        echo "Usage: $0 <threads> <repetitions> [--run-alfapang]"
        exit 1
    fi

    RUN_ALFAPANG=true
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

QUOTIENT="${ROOT_DIR}/build/quotient"

DATA_DIR="${ROOT_DIR}/data"
RESULTS_DIR="${ROOT_DIR}/results/experiment4"

ALFAPANG_DIR="${ROOT_DIR}/build/alfapang"
ALFAPANG="${ALFAPANG_DIR}/build/AlfaPang"

ALFAPANG_REPO="https://github.com/AdamCicherski/AlfaPang"
ALFAPANG_COMMIT="90bac469743920462d0fa0efc8a21a4a33513ac8"

ALFAPANG_RESULTS="${DATA_DIR}/alfapang.csv"



DATASETS=(
    ecoli50
    ecoli100
    ecoli200
    ecoli400
    ecoli800
    ecoli1600
    ecoli3412
)

declare -A K_VALUES=(
    [ecoli50]=47
    [ecoli100]=165
    [ecoli200]=217
    [ecoli400]=267
    [ecoli800]=371
    [ecoli1600]=533
    [ecoli3412]=745
)

if [[ ! -x "${QUOTIENT}" ]]; then
    echo "Error: quotient executable not found:"
    echo "  ${QUOTIENT}"
    exit 1
fi

if [[ "${RUN_ALFAPANG}" == true ]]; then
    echo "Preparing AlfaPang..."

    if [[ ! -d "${ALFAPANG_DIR}" ]]; then
        echo "Cloning AlfaPang..."
        git clone "${ALFAPANG_REPO}" "${ALFAPANG_DIR}"
    fi

    cd "${ALFAPANG_DIR}"

    git fetch --all --tags
    git checkout "${ALFAPANG_COMMIT}"

    git submodule update --init --recursive

    rm -rf build
    mkdir build

    cd build

    cmake ..
    cmake --build .

    cd "${ROOT_DIR}"

    if [[ ! -x "${ALFAPANG}" ]]; then
        echo "Error: AlfaPang executable not found:"
        echo "  ${ALFAPANG}"
        exit 1
    fi
else
    if [[ ! -f "${ALFAPANG_RESULTS}" ]]; then
        echo "Error: precomputed AlfaPang results not found:"
        echo "  ${ALFAPANG_RESULTS}"
        exit 1
    fi
fi

mkdir -p "${RESULTS_DIR}"

echo "Experiment 4"
echo "Threads:     ${THREADS}"
echo "Repetitions: ${REPETITIONS}"
echo "AlfaPang:    ${RUN_ALFAPANG}"
echo

run_quotient() {
    local dataset="$1"
    local mode="$2"
    local k="$3"
    local external_memory="$4"

    local RESULT_DIR="${RESULTS_DIR}/${dataset}/${mode}"

    mkdir -p "${RESULT_DIR}"

    for ((run = 1; run <= REPETITIONS; run++)); do
        local LOG="${RESULT_DIR}/run_${run}.log"
        local OUTPUT="${RESULT_DIR}/output.gfa"

        echo "  ${dataset} / ${mode} / run ${run}/${REPETITIONS}"

        if [[ "${external_memory}" == true ]]; then
            /usr/bin/time -v \
                bash -c '
                    cd "$1"

                    exec "$2" \
                        -i "$3" \
                        -o "$4" \
                        -k "$5" \
                        -t "$6" \
                        -e
                ' _ \
                "${RESULT_DIR}" \
                "${QUOTIENT}" \
                "${DATA_DIR}/${dataset}.fa" \
                "${OUTPUT}" \
                "${k}" \
                "${THREADS}" \
                > "${LOG}" 2>&1
        else
            /usr/bin/time -v \
                "${QUOTIENT}" \
                -i "${DATA_DIR}/${dataset}.fa" \
                -o "${OUTPUT}" \
                -k "${k}" \
                -t "${THREADS}" \
                > "${LOG}" 2>&1
        fi

        rm -f "${OUTPUT}"

        # The experiments reported in the paper cleared the filesystem
        # cache between runs. This requires elevated privileges on the host.
        #
        # sync
        # sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
    done
}

run_alfapang() {
    local dataset="$1"
    local k="$2"

    local RESULT_DIR="${RESULTS_DIR}/${dataset}/AlfaPang"

    mkdir -p "${RESULT_DIR}"

    for ((run = 1; run <= REPETITIONS; run++)); do
        local LOG="${RESULT_DIR}/run_${run}.log"
        local OUTPUT="${RESULT_DIR}/output.gfa"

        echo "  ${dataset} / AlfaPang / run ${run}/${REPETITIONS}"

        /usr/bin/time -v \
            "${ALFAPANG}" \
            "${DATA_DIR}/${dataset}.fa" \
            "${OUTPUT}" \
            "${k}" \
            > "${LOG}" 2>&1

        rm -f "${OUTPUT}"

        # The experiments reported in the paper cleared the filesystem
        # cache between runs. This requires elevated privileges on the host.
        #
        # sync
        # sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
    done
}

for dataset in "${DATASETS[@]}"; do
    k="${K_VALUES[$dataset]}"

    echo "Dataset: ${dataset} (k=${k})"

    # Main-memory quotient is omitted for ecoli3412.
    if [[ "${dataset}" != "ecoli3412" ]]; then
        run_quotient "${dataset}" "main-memory" "${k}" false
    fi

    run_quotient "${dataset}" "external-memory" "${k}" true

    if [[ "${RUN_ALFAPANG}" == true ]]; then
        run_alfapang "${dataset}" "${k}"
    fi

    echo
done

SUMMARY="${RESULTS_DIR}/summary.csv"

echo "dataset,algorithm,mode,k,total_time_median_min,peak_rss_gb" \
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
                    printf "%.6f\n", \
                        (values[NR / 2] + values[NR / 2 + 1]) / 2
            }
        '
}

get_median_measurements() {
    local RESULT_DIR="$1"

    TOTAL_TIMES=()
    RSS_VALUES=()

    for ((run = 1; run <= REPETITIONS; run++)); do
        local LOG="${RESULT_DIR}/run_${run}.log"

        local WALL_TIME="$(
            grep 'Elapsed (wall clock) time' "${LOG}" |
            sed 's/.*: //' |
            tail -n 1
        )"

        # Convert wall-clock time to minutes.
        local WALL_TIME_MIN="$(
            awk -F: '
                {
                    if (NF == 2)
                        printf "%.6f\n", ($1 * 60 + $2) / 60
                    else
                        printf "%.6f\n", ($1 * 3600 + $2 * 60 + $3) / 60
                }
            ' <<< "${WALL_TIME}"
        )"

        local RSS_KB="$(
            grep 'Maximum resident set size (kbytes)' "${LOG}" |
            awk '{print $NF}' |
            tail -n 1
        )"

        # Convert RSS from kB to GB.
        local RSS_GB="$(
            awk -v rss="${RSS_KB}" '
                BEGIN {
                    printf "%.6f\n", rss / 1024 / 1024
                }
            '
        )"

        TOTAL_TIMES+=("${WALL_TIME_MIN}")
        RSS_VALUES+=("${RSS_GB}")
    done

    MEDIAN_TIME="$(
        printf '%s\n' "${TOTAL_TIMES[@]}" |
        median
    )"

    MEDIAN_RSS="$(
        printf '%s\n' "${RSS_VALUES[@]}" |
        median
    )"
}

for dataset in "${DATASETS[@]}"; do
    k="${K_VALUES[$dataset]}"

    if [[ "${dataset}" != "ecoli3412" ]]; then
        RESULT_DIR="${RESULTS_DIR}/${dataset}/main-memory"

        get_median_measurements "${RESULT_DIR}"

        echo "${dataset},quotient,main-memory,${k},${MEDIAN_TIME},${MEDIAN_RSS}" \
            >> "${SUMMARY}"
    fi

    RESULT_DIR="${RESULTS_DIR}/${dataset}/external-memory"

    get_median_measurements "${RESULT_DIR}"

    echo "${dataset},quotient,external-memory,${k},${MEDIAN_TIME},${MEDIAN_RSS}" \
        >> "${SUMMARY}"

    if [[ "${RUN_ALFAPANG}" == true ]]; then
        RESULT_DIR="${RESULTS_DIR}/${dataset}/AlfaPang"

        get_median_measurements "${RESULT_DIR}"

        echo "${dataset},AlfaPang,historical,${k},${MEDIAN_TIME},${MEDIAN_RSS}" \
            >> "${SUMMARY}"
    else
        grep "^${dataset}," "${ALFAPANG_RESULTS}" |
            awk -F, -v dataset="${dataset}" -v k="${k}" '
                {
                    print dataset ",AlfaPang,historical," k "," $2 "," $3
                }
            ' >> "${SUMMARY}"
    fi
done

echo
echo "Experiment completed."
echo
echo "Summary:"
cat "${SUMMARY}"
echo
echo "Results stored in:"
echo "  ${RESULTS_DIR}"
