# Porting notes

This document records the design decisions of the GNU/Hurd port and the
recurring patterns used in the patches. Read it before adding patches.

## Platform model

GNU/Hurd shares glibc, the GNU userland, X11, GTK, D-Bus, NSS, etc. with
Debian GNU/Linux. What it does not share is the Linux kernel ABI. The port
therefore treats the Hurd as **a flavour of desktop Linux**:

| Layer | Linux | Hurd |
| --- | --- | --- |
| GN `current_os` / `target_os` | `"linux"` | `"hurd"` |
| GN `is_linux` | true | **true** |
| GN `is_hurd` | false | true |
| `BUILDFLAG(IS_LINUX)` | 1 | **1** |
| `BUILDFLAG(IS_HURD)` | 0 | 1 |
| PartitionAlloc `PA_BUILDFLAG(IS_HURD)` | 0 | 1 |
| Perfetto | `OS_LINUX`, `OS_LINUX_BUT_NOT_QNX` | `OS_LINUX` only (like QNX) |
| Compiler predefines | `__linux__` | `__GNU__`, `__gnu_hurd__` |
| clang / rustc triple | `x86_64-unknown-linux-gnu` | `x86_64-unknown-hurd-gnu` |

This keeps the thousands of `is_linux` / `IS_LINUX` code paths that are really
about "desktop GNU userland with X11/GTK" working unchanged, and confines the
patches to code that depends on the Linux *kernel*.

Third-party code that tests `__linux__` directly sees the Hurd as "some other
POSIX system" and usually takes a portable fallback path. Where a library has
a glibc-specific path that the Hurd needs (libc++ locale support, abseil TLS),
`defined(__GNU__)` is added next to `defined(__linux__)`.

## Patterns

Use the narrowest condition that expresses the reason, and say why in a
comment:

```c++
#if BUILDFLAG(IS_HURD)
  // GNU/Hurd has no memfd_create(); use the file-based implementation.
  return {};
#else
  ...
#endif
```

```gn
if ((is_linux && !is_hurd) || is_chromeos) {   # needs the Linux kernel
```

Prefer an existing fallback over new code: Chromium already has a portable
path for almost everything (macOS and the BSDs need one), e.g. the poll()
mode of `MessagePumpEpoll`, the `/dev/shm` file fallback for shared memory,
`DirReaderFallback`, `file_path_watcher_stub.cc`.

## Linux kernel facilities and their replacements

| Linux facility | Replacement on the Hurd |
| --- | --- |
| `epoll` | `MessagePumpEpoll`'s built-in `poll()` mode (`base/message_loop/epoll_shim_hurd.h` provides the types) |
| `eventfd` | pipe (message pump, Perfetto) |
| `futex` | PartitionAlloc / libc++abi generic locks, POSIX semaphores in the sampling profiler |
| `gettid()`, `SYS_gettid` | `pthread_self()` – libpthread thread handles are small per-process integers; `pthread_kill()` replaces `tgkill()` |
| `clone()` | `fork()`; `ForkWithFlags()` only supports the flag-free case |
| `memfd_create`, file seals | temporary files in `/dev/shm` |
| `inotify` | none (stub `FilePathWatcher`) |
| `prctl(PR_SET_NAME)` | `pthread_setname_np()` via `build/config/hurd/compat/sys/prctl.h` |
| `syscall(__NR_getrandom)` | glibc `getrandom()` |
| `getdents64` | `DirReaderFallback` |
| `CLOCK_BOOTTIME` | `CLOCK_MONOTONIC` (the Hurd does not suspend) |
| `SO_PEERCRED`, `SO_PASSCRED` | not available; peer pid is unknown |
| `SOCK_SEQPACKET` | fall back to `SOCK_DGRAM` if pflocal rejects it |
| seccomp-bpf, namespaces, setuid sandbox | not available, see below |
| cgroups, `/sys` | not available |
| `MADV_REMOVE` / `MADV_FREE` | `MADV_DONTNEED` |

`/proc` is provided by the Hurd's Linux-compatible `procfs` translator, so the
`/proc/self/stat`, `/proc/meminfo` etc. parsers are kept.

## Hurd-specific gotchas

* `O_RDONLY` is **not** 0 on the Hurd (`O_READ` = 1, `O_WRONLY` = 2,
  `O_RDWR` = 3). Code that opens files read-only by passing no access flag
  gets a descriptor without read permission.
* There is no `PATH_MAX`. The build defines `PATH_MAX=4096` for Hurd targets.
* glibc explicitly *undefines* `IOV_MAX` on the Hurd, so it cannot be defined
  from the command line; patch the user instead.
* `__MACH__` is predefined (GNU Mach). Code that uses `__MACH__` to mean
  "Apple" must also check `!defined(__GNU__)` (seen in SwiftShader,
  XNNPACK, cpu_features). Checks of the form `__ELF__ … #elif __MACH__` are
  fine because the Hurd uses ELF.
* The Hurd's `_IO()`/`_IOR()`/`_IOW()`/`_IOWR()` encode a type description
  and need an `_IOT_<struct name>` macro for every struct type. Linux-style
  `_IOWR('x', n, struct foo)` does not compile. For ioctls that only exist
  for Linux drivers (DRM, DMA-BUF, sync files) the patches spell out the
  Linux request numbers instead.
* `pthread_t` is an integer handle, not a pointer.
* Error numbers start at `0x40000000`.
* `syscall()`, `sched_getaffinity()`, `pthread_setschedparam()` and others are
  stubs that fail with `ENOSYS` (see `<gnu/stubs.h>` in the sysroot).

## Sandbox

The Linux sandbox is built on seccomp-bpf, user/PID namespaces and a setuid
helper. None of these exist on the Hurd, and the Hurd's security model
(capabilities in the form of Mach ports held by each task) would need a
completely different design. For now the port builds with
`use_seccomp_bpf = false` and Chromium must be started with `--no-sandbox`.
Multi-process isolation is kept.

## Build host

Chromium's prebuilt clang and Rust toolchains are only published for
Linux x86_64 hosts, so the build runs in a Debian container
(`docker/Dockerfile`, under Rosetta on Apple Silicon). Host tools are built
against the container's libraries, target code against the sysroot that
`scripts/make-sysroot.py` assembles from debian-ports.
