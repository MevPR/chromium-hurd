# GNU/Hurd compatibility headers

This directory is added with `-isystem` to every compilation for
`target_os = "hurd"` (see `//build/config/compiler:compiler`).

It contains small replacements for Linux-only headers that are used in many
places across Chromium and third-party code, where patching every user would
be impractical:

* `sys/prctl.h`: `prctl()` supporting `PR_SET_NAME` / `PR_GET_NAME`
  (mapped to `pthread_{set,get}name_np()`); other options fail with `EINVAL`.
* `sys/user.h`: Linux x86-64 register set layouts, used by Crashpad's ptrace
  code and the seccomp-bpf SIGSYS handler (neither runs on the Hurd).
* `sys/inotify.h`: inotify types; all functions fail with `ENOSYS`, which
  callers already handle (inotify can run out of instances on Linux too).
* `linux/netlink.h`, `linux/rtnetlink.h`, `linux/if_addr.h`: the netlink
  data structures used by `//net`'s address tracking. The Hurd has no
  netlink; `socket(AF_NETLINK, ...)` fails at runtime and
  `AddressTrackerLinux` then reports the network as online.
* `linux/sync_file.h`: DMA fence ioctls for `//third_party/libsync`. There
  are no DRM drivers, so fence descriptors never exist.

Keep this list short. Prefer fixing the user when there are only a few.
