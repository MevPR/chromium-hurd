# Chromium for Debian GNU/Hurd (amd64)

A port of the Chromium browser to [Debian GNU/Hurd](https://www.debian.org/ports/hurd/)
on x86_64 (`hurd-amd64`), maintained as a patch set against the official
Chromium release tarballs — the same model the FreeBSD and OpenBSD ports use.

> **Status: work in progress.** Chromium builds, runs headless and with a
> full browser UI under X11, and renders real web pages. See
> [docs/STATUS.md](docs/STATUS.md) for details and known issues.

![duckduckgo.com in Chromium on GNU/Hurd](docs/images/duckduckgo-x11.png)

## Layout

| Path | Purpose |
| --- | --- |
| `CHROMIUM_VERSION` | Chromium release the patches apply to |
| `overlay/` | New files copied verbatim into the Chromium tree |
| `patches/` | Modifications to existing Chromium files, applied in `series` order |
| `config/args.gn` | GN arguments for the Hurd build |
| `sysroot/packages.txt` | Debian hurd-amd64 packages that make up the sysroot |
| `scripts/` | Fetch, sysroot, toolchain, patch and build scripts |
| `docker/` | Linux x86_64 build container (runs under Rosetta on Apple Silicon) |
| `docs/` | Porting notes and status |

## Approach

* `target_os = "hurd"` is a new GN OS. Inside the build, `is_hurd` is true and
  `is_linux` stays true as well, because the Hurd shares glibc, X11, GTK and
  most of the userland with GNU/Linux. Code that depends on the *Linux
  kernel* (epoll, inotify, futex, `/proc`, seccomp, namespaces, …) is
  switched off or replaced under `is_hurd` / `BUILDFLAG(IS_HURD)`.
* The compiler targets `x86_64-unknown-hurd-gnu`; Rust uses the
  `x86_64-unknown-hurd-gnu` target with the standard library built from source.
* Chromium is cross-compiled on Linux x86_64 against a sysroot assembled
  from Debian's `hurd-amd64` packages (debian-ports).

## Building

```sh
WORK=/path/to/workdir                       # ~40 GB free space needed
docker build -t chromium-hurd-build docker/
docker run --rm -it --platform linux/amd64 \
  -v "$PWD":/port -v "$WORK":/work chromium-hurd-build sh -c '
    SRC=$(/port/scripts/fetch-source.sh /work) &&
    /port/scripts/make-sysroot.py /work/sysroot-hurd-amd64 &&
    /port/scripts/apply-patches.sh "$SRC" &&
    /port/scripts/setup-toolchain.sh "$SRC" &&
    /port/scripts/build.sh "$SRC" /work/sysroot-hurd-amd64 chrome'
```

Copy `out/hurd/` to a Hurd system (e.g. the official
[Debian GNU/Hurd amd64 image](https://cdimage.debian.org/cdimage/ports/latest/hurd-amd64/)
in QEMU) and run:

```sh
./chrome --no-sandbox --no-zygote
```

`--no-sandbox` is required (see [docs/PORTING.md](docs/PORTING.md#sandbox));
`--no-zygote` avoids the zygote's reliance on Linux-specific process and
socket features while the port is being brought up.

## License

The patches and scripts are under the same BSD-style license as Chromium
(see `LICENSE`).
