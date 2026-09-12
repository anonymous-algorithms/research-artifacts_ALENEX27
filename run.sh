#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 3 ]]; then
    echo "Usage: $0 <threads> <repetitions> <run_alfapang>"
    echo
    echo "  threads        Number of threads"
    echo "  repetitions    Number of repetitions"
    echo "  run_alfapang   true or false"
    exit 1
fi

THREADS="$1"
REPETITIONS="$2"
RUN_ALFAPANG="$3"

if [[ "${RUN_ALFAPANG}" != "true" && "${RUN_ALFAPANG}" != "false" ]]; then
    echo "Error: run_alfapang must be either 'true' or 'false'."
    exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "========================================"
echo "ALENEX27 Artifact"
echo "========================================"
echo "Threads:       ${THREADS}"
echo "Repetitions:   ${REPETITIONS}"
echo "Run AlfaPang:  ${RUN_ALFAPANG}"
echo "========================================"
echo

echo "[1/6] Building quotient..."
mkdir -p "${ROOT_DIR}/build"
cd "${ROOT_DIR}/build"

cmake ..
cmake --build .

cd "${ROOT_DIR}"

echo
echo "[2/6] Downloading and preparing datasets..."
"${ROOT_DIR}/scripts/collect_data.sh"

echo
echo "[3/6] Running Experiment 1..."
"${ROOT_DIR}/scripts/experiment1.sh" \
    "${THREADS}" \
    "${REPETITIONS}"

echo
echo "[4/6] Running Experiment 2..."
"${ROOT_DIR}/scripts/experiment2.sh" \
    "${THREADS}" \
    "${REPETITIONS}"

echo
echo "[5/6] Running Experiment 3..."
"${ROOT_DIR}/scripts/experiment3.sh" \
    "${THREADS}" \
    "${REPETITIONS}"


echo
echo "[6/6] Running Experiment 4..."
if [[ "${RUN_ALFAPANG}" == "true" ]]; then
    "${ROOT_DIR}/scripts/experiment4.sh" \
        "${THREADS}" \
        "${REPETITIONS}" \
        --run-alfapang
else
    "${ROOT_DIR}/scripts/experiment4.sh" \
        "${THREADS}" \
        "${REPETITIONS}"
fi

echo
echo "========================================"
echo "All experiments completed successfully."
echo "Results are available in:"
echo "  ${ROOT_DIR}/results/"
echo "========================================"
