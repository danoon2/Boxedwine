# Build native Linux executables and Debian packages against our oldest
# supported Ubuntu/Mint library baseline, including on Fedora Asahi hosts.
FROM docker.io/library/ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential python3 dpkg-dev binutils xz-utils ca-certificates \
        libsdl2-dev libcurl4-openssl-dev libssl-dev zlib1g-dev libgl-dev libglu1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

ENV XDG_CACHE_HOME=/tmp/boxedwine-cache
WORKDIR /tmp
