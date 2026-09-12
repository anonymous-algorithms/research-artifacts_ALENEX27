## Dataset preparation

All datasets can be downloaded and prepared automatically using:

```bash
./scripts/collect_data.sh
```

The script downloads and prepares all datasets required by the experiments. The following describes the manual procedure.

### E. coli 50, 100, 200 and 400

E. coli data used in these experiments was downloaded from [Zenodo](https://zenodo.org/).

For 50 E. coli sequences, run:

```bash
wget https://zenodo.org/records/7937947/files/ecoli50.fa.gz
gzip -d ecoli50.fa.gz
```

For the remaining datasets, download the 500-sequence dataset and use the provided name lists:

```bash
wget https://zenodo.org/records/7937947/files/ecoli500.fa.gz
gzip -d ecoli500.fa.gz

python research-artifacts_ALENEX27/scripts/preprocess_data.py \
    ecoli500.fa \
    research-artifacts_ALENEX27/data/ecoli100_names.txt \
    ecoli100.fa

python research-artifacts_ALENEX27/scripts/preprocess_data.py \
    ecoli500.fa \
    research-artifacts_ALENEX27/data/ecoli200_names.txt \
    ecoli200.fa

python research-artifacts_ALENEX27/scripts/preprocess_data.py \
    ecoli500.fa \
    research-artifacts_ALENEX27/data/ecoli400_names.txt \
    ecoli400.fa
```

### E. coli 800, 1600 and 3412

E. coli data used in these experiments was obtained from [GenBank](https://www.ncbi.nlm.nih.gov/assembly).

The original dataset was constructed on 18.06.2024 using the following criteria:

* **Date:** 18.06.2024
* **Release Type:** RefSeq
* **Query:** `txid562[Organism] AND (latest[filter] AND "complete genome"[filter] AND all[filter] NOT partial[filter]) AND latest[filter] AND all[filter] NOT anomalous[filter])`
* **Search results count:** 4534
* **Filtered out:** 1122 entries that did not have the requested Release Type or were suppressed.
* **Remaining entries:** 3412 assemblies.
* **Further filtering:** Plasmid sequences were removed.

Since June 2024, NCBI Genome and Assembly have been replaced by the NCBI Datasets CLI. We therefore recommend downloading the assemblies using the Datasets CLI.

```bash
mkdir ecoli3412_archives

while IFS= read -r line; do
    datasets download genome accession "$line" \
        --filename "ecoli3412_archives/${line}.zip"
done < research-artifacts_ALENEX27/data/ecoli_3412_accession.txt
```

After downloading the archives, extract all assemblies. Each assembly may contain multiple FASTA records, including plasmids. For each assembly, select the longest FASTA record to obtain the non-plasmid sequence. Combine the selected sequences from all 3412 assemblies into a single FASTA file:

```text
ecoli3412.fa
```

The first field of each FASTA header should contain the accession of the corresponding assembly.

Once `ecoli3412.fa` has been prepared, generate the 800- and 1600-sequence datasets using the provided name lists:

```bash
python research-artifacts_ALENEX27/scripts/preprocess_data.py \
    ecoli3412.fa \
    research-artifacts_ALENEX27/data/ecoli800_names.txt \
    ecoli800.fa

python research-artifacts_ALENEX27/scripts/preprocess_data.py \
    ecoli3412.fa \
    research-artifacts_ALENEX27/data/ecoli1600_names.txt \
    ecoli1600.fa
```

The resulting datasets are:

```text
ecoli50.fa
ecoli100.fa
ecoli200.fa
ecoli400.fa
ecoli800.fa
ecoli1600.fa
ecoli3412.fa
```
