FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    ca-certificates \
    wget \
    curl \
    pkg-config \
    unzip \
    time \
    libdivsufsort-dev \
    && rm -rf /var/lib/apt/lists/*

# Miniconda
# Miniconda
RUN wget -q https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh \
    -O /tmp/miniconda.sh \
    && bash /tmp/miniconda.sh -b -p /opt/conda \
    && rm /tmp/miniconda.sh

ENV PATH=/opt/conda/bin:$PATH

# Use conda-forge only
RUN conda config --system --remove-key channels \
    && conda config --system --add channels conda-forge \
    && conda config --system --set channel_priority strict

# NCBI datasets CLI + R + Biopython
RUN conda install -y \
    ncbi-datasets-cli \
    biopython \
    r-base \
    r-ggplot2 \
    r-gridextra \
    && conda clean -afy

WORKDIR /artifact

COPY . .

RUN chmod +x run.sh scripts/*.sh

ENTRYPOINT ["./run.sh"]
