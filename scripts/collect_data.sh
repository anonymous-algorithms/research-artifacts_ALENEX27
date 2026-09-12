#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DATA_DIR="${ROOT_DIR}/data"

ZENODO_URL="https://zenodo.org/records/7937947/files"

ECOLI50_GZ="${DATA_DIR}/ecoli50.fa.gz"
ECOLI500_GZ="${DATA_DIR}/ecoli500.fa.gz"

ECOLI50="${DATA_DIR}/ecoli50.fa"
ECOLI500="${DATA_DIR}/ecoli500.fa"
ECOLI100="${DATA_DIR}/ecoli100.fa"
ECOLI200="${DATA_DIR}/ecoli200.fa"
ECOLI400="${DATA_DIR}/ecoli400.fa"

ARCHIVES_DIR="${DATA_DIR}/ecoli3412_archives"
ACCESSION_FILE="${DATA_DIR}/ecoli_3412_accession.txt"
ECOLI3412="${DATA_DIR}/ecoli3412.fa"
ECOLI800="${DATA_DIR}/ecoli800.fa"
ECOLI1600="${DATA_DIR}/ecoli1600.fa"

PREPROCESS_SCRIPT="${ROOT_DIR}/scripts/preprocess_data.py"

echo "========================================"
echo "Collecting datasets"
echo "========================================"
echo "Repository: ${ROOT_DIR}"
echo "Data:       ${DATA_DIR}"
echo

if [[ ! -d "${DATA_DIR}" ]]; then
    echo "Error: data directory does not exist:"
    echo "  ${DATA_DIR}"
    exit 1
fi

# ---------------------------------------------------------------------------
# E. coli 50, 100, 200 and 400
# ---------------------------------------------------------------------------

echo "[1/4] Preparing E. coli 50, 100, 200 and 400..."

if [[ ! -f "${ECOLI50}" ]]; then
    if [[ ! -f "${ECOLI50_GZ}" ]]; then
        echo "Downloading ecoli50.fa.gz..."
        wget -q "${ZENODO_URL}/ecoli50.fa.gz" \
            -O "${ECOLI50_GZ}"
    fi

    echo "Extracting ecoli50.fa..."
    gzip -dk "${ECOLI50_GZ}"
fi

if [[ ! -f "${ECOLI500}" ]]; then
    if [[ ! -f "${ECOLI500_GZ}" ]]; then
        echo "Downloading ecoli500.fa.gz..."
        wget -q "${ZENODO_URL}/ecoli500.fa.gz" \
            -O "${ECOLI500_GZ}"
    fi

    echo "Extracting ecoli500.fa..."
    gzip -dk "${ECOLI500_GZ}"
fi

if [[ ! -f "${ECOLI100}" ]]; then
    echo "Creating ecoli100.fa..."
    python3 "${PREPROCESS_SCRIPT}" \
        "${ECOLI500}" \
        "${DATA_DIR}/ecoli100_names.txt" \
        "${ECOLI100}"
fi

if [[ ! -f "${ECOLI200}" ]]; then
    echo "Creating ecoli200.fa..."
    python3 "${PREPROCESS_SCRIPT}" \
        "${ECOLI500}" \
        "${DATA_DIR}/ecoli200_names.txt" \
        "${ECOLI200}"
fi

if [[ ! -f "${ECOLI400}" ]]; then
    echo "Creating ecoli400.fa..."
    python3 "${PREPROCESS_SCRIPT}" \
        "${ECOLI500}" \
        "${DATA_DIR}/ecoli400_names.txt" \
        "${ECOLI400}"
fi

# ---------------------------------------------------------------------------
# E. coli 3412
# ---------------------------------------------------------------------------

echo
echo "[2/4] Downloading E. coli 3412 assemblies..."

if [[ ! -f "${ACCESSION_FILE}" ]]; then
    echo "Error: accession file does not exist:"
    echo "  ${ACCESSION_FILE}"
    exit 1
fi

if ! command -v datasets >/dev/null 2>&1; then
    echo "Error: NCBI datasets CLI is not installed."
    exit 1
fi

mkdir -p "${ARCHIVES_DIR}"

while IFS= read -r ACCESSION; do
    [[ -z "${ACCESSION}" ]] && continue

    ZIP="${ARCHIVES_DIR}/${ACCESSION}.zip"

    if [[ -f "${ZIP}" ]]; then
        echo "Already downloaded: ${ACCESSION}"
        continue
    fi

    echo "Downloading ${ACCESSION}..."

    datasets download genome accession "${ACCESSION}" \
        --filename "${ZIP}"
done < "${ACCESSION_FILE}"

# ---------------------------------------------------------------------------
# Extract archives
# ---------------------------------------------------------------------------

echo
echo "[3/4] Extracting assemblies and creating ecoli3412.fa..."

find "${ARCHIVES_DIR}" -maxdepth 1 -type f -name "*.zip" -print0 |
while IFS= read -r -d '' ZIP; do
    ARCHIVE_NAME="$(basename "${ZIP}" .zip)"
    EXTRACT_DIR="${ARCHIVES_DIR}/${ARCHIVE_NAME}"

    if [[ -d "${EXTRACT_DIR}" ]]; then
        continue
    fi

    echo "Extracting ${ARCHIVE_NAME}..."

    mkdir -p "${EXTRACT_DIR}"
    unzip -q "${ZIP}" -d "${EXTRACT_DIR}"
done

: > "${ECOLI3412}"

find "${ARCHIVES_DIR}" -type f -name "*.fna" -print0 |
while IFS= read -r -d '' FNA; do

    echo "Processing ${FNA}..."

    awk '
        /^>/ {
            if (!found_header) {
                split($0, fields, /[[:space:]]+/)
                accession = fields[1]
                sub(/^>/, "", accession)
                found_header = 1
            }

            if (header != "") {
                if (length(seq) > max_len) {
                    max_len = length(seq)
                    max_seq = seq
                }
            }

            header = $0
            seq = ""
            next
        }

        {
            gsub(/[[:space:]]/, "", $0)
            seq = seq $0
        }

        END {
            if (header != "" && length(seq) > max_len) {
                max_seq = seq
            }

            if (max_seq != "") {
                print ">" accession
                print max_seq
            }
        }
    ' "${FNA}" >> "${ECOLI3412}"

done

# ---------------------------------------------------------------------------
# E. coli 800 and 1600
# ---------------------------------------------------------------------------

echo
echo "[4/4] Creating ecoli800.fa and ecoli1600.fa..."

if [[ ! -f "${ECOLI800}" ]]; then
    echo "Creating ecoli800.fa..."
    python3 "${PREPROCESS_SCRIPT}" \
        "${ECOLI3412}" \
        "${DATA_DIR}/ecoli800_names.txt" \
        "${ECOLI800}"
fi

if [[ ! -f "${ECOLI1600}" ]]; then
    echo "Creating ecoli1600.fa..."
    python3 "${PREPROCESS_SCRIPT}" \
        "${ECOLI3412}" \
        "${DATA_DIR}/ecoli1600_names.txt" \
        "${ECOLI1600}"
fi

echo
echo "========================================"
echo "Dataset preparation completed."
echo "========================================"
echo
echo "Generated datasets:"
echo "  ${ECOLI50}"
echo "  ${ECOLI100}"
echo "  ${ECOLI200}"
echo "  ${ECOLI400}"
echo "  ${ECOLI800}"
echo "  ${ECOLI1600}"
echo "  ${ECOLI3412}"
