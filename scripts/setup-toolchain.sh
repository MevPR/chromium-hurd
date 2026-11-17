#!/bin/sh
# Downloads the host tools a Chromium release tarball does not contain:
# Chromium's prebuilt clang and Rust toolchains, the exact Node.js release
# the build checks for, and the Go toolchain Dawn's code generators need.
# Then bootstraps gn. Must run on a Linux x86_64 host (see docker/Dockerfile).
set -eu
SRC=${1:?usage: setup-toolchain.sh SRCDIR}
cd "$SRC"

python3 tools/clang/scripts/update.py
python3 tools/rust/update_rust.py

# fetch URL SHA256 DEST: download a tarball, verify it and unpack it into DEST.
fetch() {
  tmp=$(mktemp)
  curl -fsSL -o "$tmp" "$1"
  echo "$2  $tmp" | sha256sum -c - >/dev/null
  rm -rf "$3"
  mkdir -p "$3"
  tar xf "$tmp" -C "$3" --strip-components=1
  rm -f "$tmp"
}

# Node.js: third_party/node/check_version.js requires this exact version.
NODE_VERSION=$(sed -n 's/^NODE_VERSION="\(v[0-9.]*\)"/\1/p' \
  third_party/node/update_node_binaries)
NODE_DIR=third_party/node/linux/node-linux-x64
if [ "$("$NODE_DIR/bin/node" --version 2>/dev/null)" != "$NODE_VERSION" ]; then
  base=https://nodejs.org/dist/$NODE_VERSION
  file=node-$NODE_VERSION-linux-x64.tar.xz
  sum=$(curl -fsSL "$base/SHASUMS256.txt" | awk -v f="$file" '$2 == f { print $1 }')
  fetch "$base/$file" "$sum" "$NODE_DIR"
fi

# Go: Dawn/Tint generate sources with Go programs.
GO_VERSION=$(sed -n "s/.*'dawn_go_version': 'version:[0-9]*@\([0-9.]*\)'.*/\1/p" \
  third_party/dawn/DEPS)
GO_DIR=third_party/dawn/tools/golang/linux-amd64
if [ "$("$GO_DIR/bin/go" env GOVERSION 2>/dev/null)" != "go$GO_VERSION" ]; then
  file=go$GO_VERSION.linux-amd64.tar.gz
  sum=$(curl -fsSL "https://go.dev/dl/?mode=json&include=all" |
    python3 -c "import json,sys
for r in json.load(sys.stdin):
  for f in r['files']:
    if f['filename'] == '$file': print(f['sha256'])")
  fetch "https://go.dev/dl/$file" "$sum" "$GO_DIR"
fi

if [ ! -x out/gn/gn ]; then
  LLVM=$PWD/third_party/llvm-build/Release+Asserts
  export CC=$LLVM/bin/clang CXX=$LLVM/bin/clang++ AR=$LLVM/bin/llvm-ar
  export LDFLAGS=-fuse-ld=lld
  rm -rf out/Release/gn_build
  mkdir -p out/gn
  python3 tools/gn/bootstrap/bootstrap.py --skip-generate-buildfiles \
    -o "$PWD/out/gn/gn"
fi
