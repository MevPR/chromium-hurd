// Copyright 2026 The Crashpad Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CRASHPAD_THIRD_PARTY_LSS_LSS_HURD_H_
#define CRASHPAD_THIRD_PARTY_LSS_LSS_HURD_H_

// GNU/Hurd replacement for the subset of linux_syscall_support.h that
// Crashpad uses. The Hurd has no Linux system calls, so the wrappers map to
// the equivalent libc functions. futex(2) does not exist; sys_futex() fails
// with ENOSYS, which makes the client's wait for the handler return at once.

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

typedef sigset_t kernel_sigset_t;
typedef struct timespec kernel_timespec;

#define FUTEX_WAIT_PRIVATE 128
#define FUTEX_WAKE_PRIVATE 129

static inline pid_t sys_getpid(void) {
  return getpid();
}

static inline pid_t sys_gettid(void) {
  // Matches base::PlatformThread::CurrentId() on the Hurd.
  return (pid_t)pthread_self();
}

static inline int sys_sigemptyset(kernel_sigset_t* set) {
  return sigemptyset(set);
}

static inline int sys_sigaddset(kernel_sigset_t* set, int signum) {
  return sigaddset(set, signum);
}

static inline int sys_sigprocmask(int how,
                                  const kernel_sigset_t* set,
                                  kernel_sigset_t* oldset) {
  return pthread_sigmask(how, set, oldset) == 0 ? 0 : -1;
}

static inline int sys_sigtimedwait(const kernel_sigset_t* set,
                                   siginfo_t* info,
                                   const struct timespec* timeout) {
  return sigtimedwait(set, info, timeout);
}

static inline void* sys_mmap(void* addr,
                             size_t length,
                             int prot,
                             int flags,
                             int fd,
                             off_t offset) {
  return mmap(addr, length, prot, flags, fd, offset);
}

static inline int sys_munmap(void* addr, size_t length) {
  return munmap(addr, length);
}

static inline int sys_mprotect(void* addr, size_t length, int prot) {
  return mprotect(addr, length, prot);
}

static inline int sys_prctl(int option,
                            unsigned long arg2,
                            unsigned long arg3,
                            unsigned long arg4,
                            unsigned long arg5) {
  return prctl(option, arg2, arg3, arg4, arg5);
}

static inline int sys_fallocate(int fd, int mode, off_t offset, off_t len) {
  if (mode != 0) {
    errno = EOPNOTSUPP;
    return -1;
  }
  int err = posix_fallocate(fd, offset, len);
  if (err != 0) {
    errno = err;
    return -1;
  }
  return 0;
}

static inline long sys_futex(int32_t* uaddr,
                             int op,
                             int val,
                             const kernel_timespec* timeout,
                             int32_t* uaddr2,
                             int val3) {
  (void)uaddr;
  (void)op;
  (void)val;
  (void)timeout;
  (void)uaddr2;
  (void)val3;
  errno = ENOSYS;
  return -1;
}

#endif  // CRASHPAD_THIRD_PARTY_LSS_LSS_HURD_H_
