FROM ubuntu:22.04

RUN apt-get update \
    && apt-get install -y \
        build-essential \
        libtcmalloc-minimal4 libgoogle-perftools4 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
