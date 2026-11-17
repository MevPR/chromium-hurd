// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUILD_CONFIG_HURD_COMPAT_SYS_USER_H_
#define BUILD_CONFIG_HURD_COMPAT_SYS_USER_H_

// <sys/user.h> for GNU/Hurd builds, which have no such header.
//
// Chromium uses these structures only as the layout of Linux register sets:
// for ptrace() in Crashpad and for the seccomp-bpf SIGSYS handler in the
// sandbox. Neither path can run on the Hurd, but the sizes are checked at
// compile time, so these mirror the Linux x86-64 definitions exactly.

#if !defined(__x86_64__)
#error Only x86-64 GNU/Hurd is supported
#endif

struct user_regs_struct {
  unsigned long long r15, r14, r13, r12, rbp, rbx, r11, r10, r9, r8, rax, rcx,
      rdx, rsi, rdi, orig_rax, rip, cs, eflags, rsp, ss, fs_base, gs_base, ds,
      es, fs, gs;
};

struct user_fpregs_struct {
  unsigned short cwd, swd, ftw, fop;
  unsigned long long rip, rdp;
  unsigned int mxcsr, mxcr_mask;
  unsigned int st_space[32];
  unsigned int xmm_space[64];
  unsigned int padding[24];
};

#endif  // BUILD_CONFIG_HURD_COMPAT_SYS_USER_H_
