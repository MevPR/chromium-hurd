#!/usr/bin/env python3
"""Packs the Hurd build of Chromium as a Debian binary package (hurd-amd64).

The package installs into /usr/lib/chromium and uses the system's glibc and
libraries, so it targets a Debian GNU/Hurd system that matches the sysroot
(unstable). Its Depends are derived from the DT_NEEDED entries of the
shipped ELF files, mapped to the sysroot packages that provide them.

Run inside the build container:

  scripts/make-deb.py SRCDIR SYSROOT OUTDIR

SYSROOT must be the directory created by make-sysroot.py; the .debs and the
manifest that make-sysroot.py leaves next to it are used for the mapping.
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile

PACKAGE = 'chromium'
ARCH = 'hurd-amd64'
LIBDIR = 'usr/lib/chromium'
MAINTAINER = 'Magnus Reimann <magnus.reimann@mailbox.org>'
DEBIAN_REVISION = '0hurd1'

# Files copied from out/hurd into LIBDIR besides chrome itself.
RESOURCES = [
    'chrome_100_percent.pak',
    'chrome_200_percent.pak',
    'headless_command_resources.pak',
    'resources.pak',
    'icudtl.dat',
    'snapshot_blob.bin',
    'v8_context_snapshot.bin',
    'vk_swiftshader_icd.json',
]
# Shared libraries built with Chromium; ANGLE loads libvulkan.so.1 and the
# SwiftShader ICD from its own directory.
LIBRARIES = [
    'libEGL.so',
    'libGLESv2.so',
    'libvk_swiftshader.so',
    'libvulkan.so.1',
]

WRAPPER = '''#!/bin/sh
# Chromium for GNU/Hurd. The Linux sandbox (seccomp-bpf, namespaces) and the
# zygote do not exist on the Hurd, see /usr/share/doc/chromium/README.Hurd.
exec /usr/lib/chromium/chrome --no-sandbox --no-zygote "$@"
'''

DESKTOP = '''[Desktop Entry]
Version=1.0
Name=Chromium
GenericName=Web Browser
Comment=Access the Internet
Exec=chromium %U
Terminal=false
Icon=chromium
Type=Application
Categories=Network;WebBrowser;
MimeType=text/html;text/xml;application/xhtml+xml;x-scheme-handler/http;x-scheme-handler/https;
StartupWMClass=chromium-browser
'''

README_HURD = '''Chromium for Debian GNU/Hurd
============================

This package is built from the patch set at
https://github.com/MevPR/chromium-hurd.

The Linux sandbox relies on seccomp-bpf, namespaces and a setuid helper,
none of which exist on the Hurd. /usr/bin/chromium therefore starts the
browser with --no-sandbox and --no-zygote. Multi-process isolation between
the browser, renderer, GPU and utility processes is kept, but renderers are
not confined.

Without OpenGL (e.g. under Xvfb) the GPU process falls back to software
compositing. Pass --use-angle=swiftshader --enable-unsafe-swiftshader to use
SwiftShader instead (needed for WebGL).

On slow (emulated) machines, add --ipc-connection-timeout=120 and
--disable-gpu-watchdog.
'''


def run(*args, **kwargs):
  return subprocess.run(args, check=True, text=True, capture_output=True,
                        **kwargs).stdout


def is_elf(path):
  with open(path, 'rb') as f:
    return f.read(4) == b'\x7fELF'


def needed_libraries(readelf, path):
  out = run(readelf, '--dynamic', path)
  return re.findall(r'\(NEEDED\)\s+Shared library: \[([^\]]+)\]', out)


def soname_providers(sysroot):
  """Maps library file names to (package, version) from the sysroot debs."""
  debs_dir = sysroot.rstrip('/') + '.debs'
  manifest = sysroot.rstrip('/') + '.manifest'
  versions = {}
  with open(manifest) as f:
    for line in f:
      name, version = line.split()
      versions[name] = version
  providers = {}
  for deb in sorted(os.listdir(debs_dir)):
    if not deb.endswith('.deb'):
      continue
    package = deb.split('_')[0]
    listing = run('dpkg-deb', '--contents', os.path.join(debs_dir, deb))
    for line in listing.splitlines():
      path = line.split()[5] if len(line.split()) > 5 else ''
      if re.search(r'/lib/(x86_64-gnu/)?[^/]+\.so[.0-9]*$', path):
        providers.setdefault(os.path.basename(path),
                             (package, versions.get(package)))
  return providers


def upstream_version(src):
  values = {}
  with open(os.path.join(src, 'chrome', 'VERSION')) as f:
    for line in f:
      key, value = line.strip().split('=')
      values[key] = value
  return '{MAJOR}.{MINOR}.{BUILD}.{PATCH}'.format(**values)


def main():
  parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
  parser.add_argument('src', help='Chromium source tree with out/hurd')
  parser.add_argument('sysroot', help='sysroot created by make-sysroot.py')
  parser.add_argument('outdir', help='directory for the .deb')
  args = parser.parse_args()

  src = os.path.abspath(args.src)
  build = os.path.join(src, 'out', 'hurd')
  llvm = os.path.join(src, 'third_party', 'llvm-build', 'Release+Asserts',
                      'bin')
  strip = os.path.join(llvm, 'llvm-strip')
  readelf = os.path.join(llvm, 'llvm-readelf')
  version = upstream_version(src) + '-' + DEBIAN_REVISION

  stage = tempfile.mkdtemp()
  try:
    libdir = os.path.join(stage, LIBDIR)
    os.makedirs(os.path.join(libdir, 'locales'))
    run(strip, '--strip-all', '-o', os.path.join(libdir, 'chrome'),
        os.path.join(build, 'chrome'))
    for name in LIBRARIES:
      run(strip, '--strip-unneeded', '-o', os.path.join(libdir, name),
          os.path.join(build, name))
    for name in RESOURCES:
      shutil.copy(os.path.join(build, name), libdir)
    for name in os.listdir(os.path.join(build, 'locales')):
      if name.endswith('.pak'):
        shutil.copy(os.path.join(build, 'locales', name),
                    os.path.join(libdir, 'locales'))

    bindir = os.path.join(stage, 'usr', 'bin')
    os.makedirs(bindir)
    with open(os.path.join(bindir, 'chromium'), 'w') as f:
      f.write(WRAPPER)

    share = os.path.join(stage, 'usr', 'share')
    os.makedirs(os.path.join(share, 'applications'))
    with open(os.path.join(share, 'applications', 'chromium.desktop'),
              'w') as f:
      f.write(DESKTOP)
    theme = os.path.join(src, 'chrome', 'app', 'theme', 'chromium')
    for size in (16, 24, 48, 64, 128, 256):
      icondir = os.path.join(share, 'icons', 'hicolor', f'{size}x{size}',
                             'apps')
      os.makedirs(icondir)
      shutil.copy(os.path.join(theme, f'product_logo_{size}.png'),
                  os.path.join(icondir, 'chromium.png'))
    docdir = os.path.join(share, 'doc', PACKAGE)
    os.makedirs(docdir)
    with open(os.path.join(docdir, 'README.Hurd'), 'w') as f:
      f.write(README_HURD)
    shutil.copy(os.path.join(src, 'LICENSE'),
                os.path.join(docdir, 'copyright'))

    for root, dirs, files in os.walk(stage):
      for d in dirs:
        os.chmod(os.path.join(root, d), 0o755)
      for name in files:
        os.chmod(os.path.join(root, name), 0o644)
    os.chmod(os.path.join(bindir, 'chromium'), 0o755)
    os.chmod(os.path.join(libdir, 'chrome'), 0o755)

    # Depends from DT_NEEDED, minus the libraries shipped in the package.
    shipped = set(LIBRARIES)
    needed = set()
    for root, _, files in os.walk(libdir):
      for name in files:
        path = os.path.join(root, name)
        if is_elf(path):
          needed.update(needed_libraries(readelf, path))
    providers = soname_providers(args.sysroot)
    depends = {}
    missing = []
    for lib in sorted(needed - shipped):
      if lib not in providers:
        missing.append(lib)
        continue
      package, pkg_version = providers[lib]
      # Strip the Debian revision and binNMU suffix for a >= constraint.
      depends[package] = re.sub(r'-[^-]*$', '', pkg_version or '')
    if missing:
      sys.exit('no package provides: ' + ', '.join(missing))

    size_kb = sum(
        os.path.getsize(os.path.join(root, name))
        for root, _, files in os.walk(stage) for name in files) // 1024
    depends_field = ', '.join(
        f'{p} (>= {v})' if v else p for p, v in sorted(depends.items()))
    control_dir = os.path.join(stage, 'DEBIAN')
    os.makedirs(control_dir)
    with open(os.path.join(control_dir, 'control'), 'w') as f:
      f.write(f'''Package: {PACKAGE}
Version: {version}
Architecture: {ARCH}
Maintainer: {MAINTAINER}
Installed-Size: {size_kb}
Depends: {depends_field}
Recommends: fonts-liberation, libgtk-3-0t64, xdg-utils
Provides: www-browser
Section: web
Priority: optional
Homepage: https://github.com/MevPR/chromium-hurd
Description: web browser (GNU/Hurd port)
 Chromium is an open-source browser project that aims to build a safer,
 faster, and more stable way for all Internet users to experience the web.
 .
 This package contains Chromium built for Debian GNU/Hurd. The Linux sandbox
 is not available; see /usr/share/doc/chromium/README.Hurd.
''')

    os.makedirs(args.outdir, exist_ok=True)
    deb = os.path.join(args.outdir, f'{PACKAGE}_{version}_{ARCH}.deb')
    run('dpkg-deb', '--root-owner-group', '-Zxz', '--build', stage, deb)
    print(deb)
  finally:
    shutil.rmtree(stage)


if __name__ == '__main__':
  main()
