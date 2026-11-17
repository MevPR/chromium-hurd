# Developing the port

## Working tree

Patches are developed in an extracted release tarball that is turned into a
git repository, so `git diff` shows exactly what the port changes:

```sh
SRC=$(scripts/fetch-source.sh "$WORK")
cd "$SRC" && git init -q && git add -A && git commit -qm "Chromium $(cat CHROMIUM_VERSION)"
cd - && scripts/apply-patches.sh "$SRC"
```

Edit files in `$SRC`, then regenerate the patch set:

```sh
scripts/export-patches.py "$SRC"     # rewrites patches/*.patch and patches/series
```

New files go into `overlay/` (same relative path as in the Chromium tree);
`apply-patches.sh` copies them in before applying the patches.

## Build loop

```sh
docker run --rm --platform linux/amd64 \
  -v "$PWD":/port -v "$WORK":/work chromium-hurd-build \
  /port/scripts/build.sh "/work/chromium-$(cat CHROMIUM_VERSION)" \
                         /work/sysroot-hurd-amd64 base
```

Useful intermediate targets, in order of size: `base`, `check_example`
(smallest executable linking `//base`), `base_unittests`, `content_shell`,
`chrome`. Run ninja with `-k 0` to collect all errors of a round at once.

## Testing on the Hurd

Boot the Debian GNU/Hurd amd64 image in QEMU (see `scripts/run-vm.sh`), copy
`out/hurd` over SSH (port 2222 on the host) and run binaries there. The image
must be upgraded to unstable (`apt full-upgrade`) because the sysroot is built
from unstable and binaries require the matching glibc.

On Apple Silicon, use an AHCI disk (`-device ahci`); the Hurd's rumpdisk
reports I/O errors on QEMU's emulated PIIX IDE controller under TCG.
