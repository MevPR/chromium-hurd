#!/usr/bin/env python3
"""Builds a Debian GNU/Hurd amd64 sysroot for cross-compiling Chromium.

Downloads the seed packages from sysroot/packages.txt plus their transitive
Depends/Pre-Depends from debian-ports, unpacks them with dpkg-deb and turns
absolute symlinks into relative ones so the tree can be used with --sysroot.

Usage: make-sysroot.py OUTPUT_DIR [--mirror URL] [--suite sid]
"""

import argparse
import gzip
import lzma
import os
import re
import subprocess
import sys
import urllib.request

ARCH = 'hurd-amd64'
DEFAULT_MIRROR = 'https://deb.debian.org/debian-ports'
SUITES = ['sid', 'unreleased']
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Packages that are pulled in by dependencies but are useless (or harmful)
# in a sysroot: runtime daemons, interpreters, documentation, and the
# pseudo-essential base that only matters on a running system.
SKIP = {
    'base-files', 'debconf', 'dpkg', 'perl-base', 'perl', 'python3',
    'python3-minimal', 'sensible-utils', 'install-info', 'dbus',
    'dbus-daemon', 'dbus-system-bus-common', 'dbus-session-bus-common',
    'dbus-bin', 'adduser', 'passwd', 'login', 'tzdata',
    'hicolor-icon-theme', 'adwaita-icon-theme', 'gtk-update-icon-cache',
    'fontconfig-config', 'fonts-dejavu-core', 'fonts-dejavu-mono',
    'libgtk-3-common', 'libgtk-3-bin', 'dconf-service', 'hurd',
    'gnumach-image-1.8-amd64', 'sysvinit-core', 'mount',
}


def fetch(url):
    with urllib.request.urlopen(url) as r:
        return r.read()


def load_index(mirror):
    packages = {}
    for suite in SUITES:
        url = f'{mirror}/dists/{suite}/main/binary-{ARCH}/Packages.xz'
        print(f'Fetching {url}', file=sys.stderr)
        text = lzma.decompress(fetch(url)).decode('utf-8', 'replace')
        for stanza in text.split('\n\n'):
            fields = {}
            key = None
            for line in stanza.splitlines():
                if line.startswith((' ', '\t')) and key:
                    fields[key] += '\n' + line
                elif ':' in line:
                    key, _, value = line.partition(':')
                    fields[key] = value.strip()
            name = fields.get('Package')
            if not name:
                continue
            # The first suite wins; 'unreleased' only fills gaps.
            packages.setdefault(name, fields)
            for prov in split_deps(fields.get('Provides', '')):
                packages.setdefault('virtual:' + prov[0], fields)
    return packages


def split_deps(value):
    """Returns a list of alternatives lists of package names."""
    result = []
    for group in value.split(','):
        group = group.strip()
        if not group:
            continue
        alts = []
        for alt in group.split('|'):
            name = re.sub(r'\(.*?\)|\[.*?\]|<.*?>', '', alt).strip()
            name = name.split(':')[0]
            if name:
                alts.append(name)
        result.append(alts)
    return result


def resolve(packages, seeds):
    selected = {}
    queue = list(seeds)
    while queue:
        name = queue.pop()
        if name in SKIP or name in selected:
            continue
        fields = packages.get(name) or packages.get('virtual:' + name)
        if not fields:
            raise SystemExit(f'Package not found for {ARCH}: {name}')
        real = fields['Package']
        if real in selected or real in SKIP:
            continue
        selected[real] = fields
        deps = fields.get('Depends', '') + ',' + fields.get('Pre-Depends', '')
        for alts in split_deps(deps):
            if any(a in selected or a in SKIP for a in alts):
                continue
            pick = next((a for a in alts if a in packages), None)
            pick = pick or next(
                (a for a in alts if 'virtual:' + a in packages), None)
            if pick is None:
                raise SystemExit(f'{real}: unresolvable dependency {alts}')
            queue.append(pick)
    return selected


def relativize_symlinks(root):
    for dirpath, dirnames, filenames in os.walk(root):
        for name in dirnames + filenames:
            path = os.path.join(dirpath, name)
            if not os.path.islink(path):
                continue
            target = os.readlink(path)
            if not target.startswith('/'):
                continue
            new = os.path.relpath(os.path.join(root, target.lstrip('/')),
                                  dirpath)
            os.unlink(path)
            os.symlink(new, path)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('output')
    parser.add_argument('--mirror', default=DEFAULT_MIRROR)
    parser.add_argument('--packages',
                        default=os.path.join(ROOT, 'sysroot', 'packages.txt'))
    args = parser.parse_args()

    with open(args.packages) as f:
        seeds = [l.split('#')[0].strip() for l in f]
    seeds = [s for s in seeds if s]

    packages = load_index(args.mirror)
    selected = resolve(packages, seeds)
    print(f'{len(selected)} packages selected', file=sys.stderr)

    out = os.path.abspath(args.output)
    cache = os.path.join(out + '.debs')
    os.makedirs(out, exist_ok=True)
    os.makedirs(cache, exist_ok=True)

    with open(out + '.manifest', 'w') as manifest:
        for name in sorted(selected):
            fields = selected[name]
            manifest.write(f'{name} {fields["Version"]}\n')
            deb = os.path.join(cache, os.path.basename(fields['Filename']))
            if not os.path.exists(deb):
                print(f'  {name} {fields["Version"]}', file=sys.stderr)
                data = fetch(f'{args.mirror}/{fields["Filename"]}')
                with open(deb + '.tmp', 'wb') as f:
                    f.write(data)
                os.rename(deb + '.tmp', deb)
            subprocess.check_call(['dpkg-deb', '-x', deb, out])

    # Documentation and locale data are not needed.
    for sub in ('usr/share/doc', 'usr/share/man', 'usr/share/locale',
                'usr/share/info', 'usr/share/help'):
        subprocess.call(['rm', '-rf', os.path.join(out, sub)])
    # Merged-/usr layout (normally created by base-files); libc.so refers to
    # /lib/ld-x86-64.so.1.
    for d in ('lib', 'bin', 'sbin'):
        if not os.path.lexists(os.path.join(out, d)):
            os.symlink(os.path.join('usr', d), os.path.join(out, d))
    relativize_symlinks(out)
    print(f'Sysroot ready in {out}', file=sys.stderr)


if __name__ == '__main__':
    main()
