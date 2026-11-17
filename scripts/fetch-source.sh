#!/bin/sh
# Downloads and verifies the official Chromium source tarball for the version
# in CHROMIUM_VERSION. Usage: fetch-source.sh WORKDIR
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
WORK=${1:?usage: fetch-source.sh WORKDIR}
VERSION=$(cat "$ROOT/CHROMIUM_VERSION")
BASE=https://commondatastorage.googleapis.com/chromium-browser-official
TARBALL=chromium-$VERSION-lite.tar.xz

mkdir -p "$WORK/dl"
cd "$WORK/dl"
curl -fL -C - -o "$TARBALL" "$BASE/$TARBALL"
curl -fsSL -o "$TARBALL.hashes" "$BASE/$TARBALL.hashes"
expected=$(awk '$1 == "sha256" { print $2 }' "$TARBALL.hashes")
actual=$( (sha256sum "$TARBALL" 2>/dev/null || shasum -a 256 "$TARBALL") | cut -d' ' -f1)
[ "$expected" = "$actual" ] || { echo "sha256 mismatch for $TARBALL" >&2; exit 1; }

cd "$WORK"
[ -d "chromium-$VERSION" ] || tar xf "dl/$TARBALL"
echo "$WORK/chromium-$VERSION"
