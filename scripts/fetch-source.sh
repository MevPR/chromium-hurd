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

# The lite tarball lacks the test fonts that test targets such as
# base_unittests depend on; fetch them from the bucket named in DEPS.
SRC=$WORK/chromium-$VERSION
FONTS=$SRC/third_party/test_fonts/test_fonts
if [ ! -e "$FONTS/Ahem.ttf" ]; then
  entry=$(grep -A10 "'src/third_party/test_fonts/test_fonts'" "$SRC/DEPS")
  object=$(echo "$entry" | sed -n "s/.*'object_name': '\([0-9a-f]*\)'.*/\1/p")
  sha=$(echo "$entry" | sed -n "s/.*'sha256sum': '\([0-9a-f]*\)'.*/\1/p")
  curl -fsSL -o dl/test_fonts.tar.gz \
    "https://storage.googleapis.com/chromium-fonts/$object"
  actual=$( (sha256sum dl/test_fonts.tar.gz 2>/dev/null ||
             shasum -a 256 dl/test_fonts.tar.gz) | cut -d' ' -f1)
  [ "$sha" = "$actual" ] || { echo "sha256 mismatch for test fonts" >&2; exit 1; }
  mkdir -p "$FONTS" && tar xzf dl/test_fonts.tar.gz -C "$FONTS"
fi
echo "$SRC"
