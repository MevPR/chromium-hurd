#!/usr/bin/env python3
"""Regenerates patches/ from a patched Chromium tree.

The tree must be a git repository whose HEAD is the pristine release (see
docs/DEVELOPING.md). Modified tracked files are grouped into one patch per
component (top-level directory, or third_party/<name>) and patches/series is
rewritten. New files belong in overlay/ and are not exported here.

Usage: export-patches.py SRCDIR
"""

import os
import subprocess
import sys
from collections import defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PATCHES = os.path.join(ROOT, 'patches')

# Applied first, in this order; everything else follows alphabetically.
FIRST = ['build', 'base', 'v8']


def component(path):
    parts = path.split('/')
    if parts[0] in ('third_party', 'v8') and len(parts) > 2:
        if parts[0] == 'third_party':
            return 'third_party-' + parts[1]
    return parts[0]


def main():
    src = sys.argv[1]
    files = subprocess.check_output(
        ['git', 'diff', '--name-only', 'HEAD'], cwd=src,
        text=True).split()
    groups = defaultdict(list)
    for f in files:
        groups[component(f)].append(f)

    for name in os.listdir(PATCHES):
        if name.endswith('.patch'):
            os.unlink(os.path.join(PATCHES, name))

    order = [g for g in FIRST if g in groups]
    order += sorted(g for g in groups if g not in FIRST)
    for g in order:
        diff = subprocess.check_output(
            ['git', 'diff', '--no-color', '--no-ext-diff', 'HEAD', '--'] +
            sorted(groups[g]), cwd=src)
        with open(os.path.join(PATCHES, f'{g}.patch'), 'wb') as f:
            f.write(diff)
    with open(os.path.join(PATCHES, 'series'), 'w') as f:
        f.write('# Applied in order by scripts/apply-patches.sh\n')
        for g in order:
            f.write(f'{g}.patch\n')
    print(f'{len(order)} patches, {len(files)} files')


if __name__ == '__main__':
    main()
