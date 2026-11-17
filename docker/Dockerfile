# Build environment for cross-compiling Chromium to Debian GNU/Hurd amd64.
# The host side is Linux x86_64 because Chromium's prebuilt clang/rust
# toolchains are only published for that host. On Apple Silicon this runs
# under Rosetta (docker run --platform linux/amd64).
FROM --platform=linux/amd64 debian:sid

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
      --no-install-recommends \
      build-essential ca-certificates curl dpkg-dev file git gperf \
      libglib2.0-dev libnss3-dev libgtk-3-dev libx11-dev libxkbcommon-dev \
      libdrm-dev libcups2-dev libpulse-dev libkrb5-dev libpci-dev \
      libxshmfence-dev libxtst-dev libxrandr-dev libxdamage-dev \
      libxcomposite-dev libgbm-dev libasound2-dev libudev-dev \
      ninja-build nodejs pkg-config python3 python3-setuptools \
      qemu-utils xz-utils zstd \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
