#!/bin/sh
# Copies overlay/ into the Chromium tree and applies patches/series.
# Usage: apply-patches.sh SRCDIR
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
SRC=${1:?usage: apply-patches.sh SRCDIR}

cp -R "$ROOT/overlay/." "$SRC/"
grep -v '^\s*#' "$ROOT/patches/series" | while read -r p; do
  [ -n "$p" ] || continue
  echo "Applying $p"
  patch -d "$SRC" -p1 --forward --no-backup-if-mismatch < "$ROOT/patches/$p"
done
