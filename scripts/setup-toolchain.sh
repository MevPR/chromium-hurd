#!/bin/sh
# Downloads Chromium's prebuilt clang and rust toolchains and bootstraps gn.
# Must run on a Linux x86_64 host (see docker/Dockerfile).
set -eu
SRC=${1:?usage: setup-toolchain.sh SRCDIR}
cd "$SRC"
python3 tools/clang/scripts/update.py
python3 tools/rust/update_rust.py
if [ ! -x out/gn/gn ]; then
  LLVM=$PWD/third_party/llvm-build/Release+Asserts
  export CC=$LLVM/bin/clang CXX=$LLVM/bin/clang++ AR=$LLVM/bin/llvm-ar
  export LDFLAGS=-fuse-ld=lld
  rm -rf out/Release/gn_build
  mkdir -p out/gn
  python3 tools/gn/bootstrap/bootstrap.py --skip-generate-buildfiles \
    -o "$PWD/out/gn/gn"
fi
