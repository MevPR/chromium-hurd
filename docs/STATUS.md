# Status

Chromium 157.0.8103.78 → Debian GNU/Hurd amd64. Last updated: 2026-11-18.

## Milestones

| Milestone | State |
| --- | --- |
| Hurd sysroot from debian-ports (308 packages) | ✅ |
| `gn gen` with `target_os = "hurd"` | ✅ |
| `//base` compiles | ✅ |
| First linked executable (`check_example`): ELF for GNU/Hurd, `ld-x86-64.so.1`, `libc.so.0.3` | ✅ |
| `check_example` runs on Debian GNU/Hurd (QEMU): logging, `CHECK`, stack-trace handler work | ✅ |
| `chrome` compiles and links (ELF for GNU/Hurd) | ✅ |
| `chrome --version` runs on Debian GNU/Hurd | ✅ |
| `chrome --headless --dump-dom` renders a page (browser, renderer, network and GPU processes, Mojo, Blink) | ✅ |
| Clean shutdown (PartitionAlloc TLS during libc thread teardown) | ✅ |
| GPU process with SwiftShader (SwANGLE), WebGL in headless mode | ✅ |
| `base_unittests` pass on the Hurd | ☐ |
| `chrome --no-sandbox` starts with X11 (Xvfb), full browser UI | ✅ |
| https://duckduckgo.com renders correctly over the network | ✅ |
| Debian packaging | ☐ |

## Disabled features

| Feature | Reason |
| --- | --- |
| seccomp-bpf / namespace sandbox | Linux kernel only; run with `--no-sandbox` |
| Crashpad (compiled, handler not started) | needs ptrace and `/proc/<pid>/mem` |
| Wayland, GBM, VA-API, V4L2 | not available on the Hurd |
| ALSA, udev, libinput, evdev | not available on the Hurd |
| PipeWire, remoting | not available on the Hurd |
| Widevine | no Hurd CDM |
| inotify-based file watching | no file change notification API on the Hurd |
| netlink network change tracking | network is always reported as online |
| Mojo shared-memory channel upgrade | needs memfd and futex |
| GBM / native pixmaps on X11 | no DRM render nodes |
| cpuinfo CPU detection (XNNPACK) | Linux backend reads sysfs; Hurd backend TODO |

## Known runtime issues

* Dawn (WebGPU) finds no Vulkan driver of its own and logs warnings; WebGPU
  is unavailable.
* With `--use-angle=swiftshader --enable-unsafe-swiftshader` under X11, WebGL
  contexts are created (SwiftShader Vulkan), but the canvas is displayed
  blank; under investigation.
* `CountResidentBytes` (`mincore`) is not implemented on the Hurd; memory
  dumps lack resident sizes.

## Known runtime risks

* `SOCK_SEQPACKET` support in pflocal (falls back to `SOCK_DGRAM`).
* Peer credentials (`SO_PEERCRED`) are unavailable; code paths that need
  them fail closed.
* V8's WebAssembly trap handler relies on SIGSEGV handling with precise
  `ucontext` contents.
* Under QEMU without KVM, child processes can miss the 15 s IPC connection
  timeout and terminate themselves (seen as "Network service crashed").
  `run-chrome` passes `--ipc-connection-timeout=120`. The GPU watchdog
  likewise kills the GPU process while SwiftShader renders, so `run-chrome`
  also passes `--disable-gpu-watchdog`.
* Performance under QEMU emulation is too slow for real browsing; real
  hardware or KVM is needed.
