#!/bin/sh
# Configures and builds Chromium for Debian GNU/Hurd amd64.
# Usage: build.sh SRCDIR SYSROOT [ninja targets...]   (default target: chrome)
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
SRC=${1:?usage: build.sh SRCDIR SYSROOT [targets]}
SYSROOT=$(cd "${2:?missing SYSROOT}" && pwd)
shift 2
OUT=out/hurd
cd "$SRC"
mkdir -p "$OUT"
{ cat "$ROOT/config/args.gn"; echo "target_sysroot = \"$SYSROOT\""; } > "$OUT/args.gn.new"
cmp -s "$OUT/args.gn.new" "$OUT/args.gn" 2>/dev/null || mv "$OUT/args.gn.new" "$OUT/args.gn"
rm -f "$OUT/args.gn.new"
out/gn/gn gen "$OUT"
ninja -C "$OUT" ${@:-chrome}
