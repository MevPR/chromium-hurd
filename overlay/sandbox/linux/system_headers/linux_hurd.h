// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SANDBOX_LINUX_SYSTEM_HEADERS_LINUX_HURD_H_
#define SANDBOX_LINUX_SYSTEM_HEADERS_LINUX_HURD_H_

// Linux UAPI definitions that GNU/Hurd's headers lack.
//
// The Hurd builds the Linux sandbox code so that the rest of Chromium keeps
// its interfaces, but none of it can engage there: seccomp, namespaces,
// clone() and the setuid helper do not exist, and the runtime checks report
// the sandbox as unavailable (Chrome must be started with --no-sandbox).
// Seccomp policies are expressed in Linux values, so these are Linux values
// too. This header is empty on Linux.

#if defined(__GNU__)

#include <sys/types.h>

// <sched.h>
#ifndef CLONE_VM
#define CLONE_VM 0x00000100
#define CLONE_FS 0x00000200
#define CLONE_FILES 0x00000400
#define CLONE_SIGHAND 0x00000800
#define CLONE_VFORK 0x00004000
#define CLONE_PARENT 0x00008000
#define CLONE_THREAD 0x00010000
#define CLONE_NEWNS 0x00020000
#define CLONE_SYSVSEM 0x00040000
#define CLONE_SETTLS 0x00080000
#define CLONE_PARENT_SETTID 0x00100000
#define CLONE_CHILD_CLEARTID 0x00200000
#define CLONE_DETACHED 0x00400000
#define CLONE_UNTRACED 0x00800000
#define CLONE_CHILD_SETTID 0x01000000
#define CLONE_NEWCGROUP 0x02000000
#define CLONE_NEWUTS 0x04000000
#define CLONE_NEWIPC 0x08000000
#define CLONE_NEWUSER 0x10000000
#define CLONE_NEWPID 0x20000000
#define CLONE_NEWNET 0x40000000
#define CLONE_IO 0x80000000
#endif

// <fcntl.h>
#ifndef O_DIRECT
#define O_DIRECT 040000
#endif
#ifndef F_ADD_SEALS
#define F_ADD_SEALS 1033
#define F_GET_SEALS 1034
#define F_SEAL_SEAL 0x0001
#define F_SEAL_SHRINK 0x0002
#define F_SEAL_GROW 0x0004
#define F_SEAL_WRITE 0x0008
#endif

// <sys/mman.h>
#ifndef MADV_REMOVE
#define MADV_REMOVE 9
#endif
#ifndef MAP_POPULATE
#define MAP_POPULATE 0x08000
#endif

// <sys/socket.h>: the Hurd has no SO_PASSCRED/SCM_CREDENTIALS. Credentials
// are never received; the SCM value avoids the Hurd's own SCM_* types.
struct ucred {
  pid_t pid;
  uid_t uid;
  gid_t gid;
};
#ifndef SO_PASSCRED
#define SO_PASSCRED 16
#endif
#ifndef SCM_CREDENTIALS
#define SCM_CREDENTIALS 0x7f02
#endif

// <sys/ptrace.h>
#ifndef PTRACE_GET_THREAD_AREA
#define PTRACE_GET_THREAD_AREA 25
#endif

// <linux/kcmp.h>
#ifndef KCMP_FILE
#define KCMP_FILE 0
#endif

// <linux/net.h>: socketcall(2) multiplexer numbers.
#ifndef SYS_SOCKET
#define SYS_SOCKET 1
#define SYS_BIND 2
#define SYS_CONNECT 3
#define SYS_LISTEN 4
#define SYS_ACCEPT 5
#define SYS_GETSOCKNAME 6
#define SYS_GETPEERNAME 7
#define SYS_SOCKETPAIR 8
#define SYS_SEND 9
#define SYS_RECV 10
#define SYS_SENDTO 11
#define SYS_RECVFROM 12
#define SYS_SHUTDOWN 13
#define SYS_SETSOCKOPT 14
#define SYS_GETSOCKOPT 15
#define SYS_SENDMSG 16
#define SYS_RECVMSG 17
#define SYS_ACCEPT4 18
#define SYS_RECVMMSG 19
#define SYS_SENDMMSG 20
#endif

#endif  // defined(__GNU__)

#endif  // SANDBOX_LINUX_SYSTEM_HEADERS_LINUX_HURD_H_
