// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUILD_CONFIG_HURD_COMPAT_SYS_PRCTL_H_
#define BUILD_CONFIG_HURD_COMPAT_SYS_PRCTL_H_

// Minimal prctl(2) for GNU/Hurd, which has no such call. Chromium mostly uses
// it to name threads; that is mapped onto pthread_{set,get}name_np().
// PR_{GET,SET}_DUMPABLE report/accept the Hurd's fixed behaviour. All other
// options fail with EINVAL, which callers already handle because they must
// cope with older Linux kernels.
//
// This directory is only on the include path when building for the Hurd
// (see //build/config/compiler:compiler).

#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>

#define PR_SET_PDEATHSIG 1
#define PR_GET_DUMPABLE 3
#define PR_SET_DUMPABLE 4
#define PR_SET_NAME 15
#define PR_GET_NAME 16
#define PR_SET_TIMERSLACK 29
#define PR_SET_NO_NEW_PRIVS 38
#define PR_GET_NO_NEW_PRIVS 39
#define PR_SET_PTRACER 0x59616d61
#define PR_SET_PTRACER_ANY ((unsigned long)-1)

// Thread names are limited to 16 bytes including the terminator on Linux.
#define HURD_COMPAT_PR_NAME_LEN 16

static inline int prctl(int option, ...) {
  va_list ap;
  va_start(ap, option);
  int result = -1;
  switch (option) {
    case PR_SET_NAME: {
      const char* name = va_arg(ap, const char*);
      char buf[HURD_COMPAT_PR_NAME_LEN];
      strncpy(buf, name, sizeof(buf) - 1);
      buf[sizeof(buf) - 1] = '\0';
      int err = pthread_setname_np(pthread_self(), buf);
      if (err == 0) {
        result = 0;
      } else {
        errno = err;
      }
      break;
    }
    case PR_GET_NAME: {
      char* name = va_arg(ap, char*);
      int err = pthread_getname_np(pthread_self(), name,
                                   HURD_COMPAT_PR_NAME_LEN);
      if (err == 0) {
        result = 0;
      } else {
        errno = err;
      }
      break;
    }
    case PR_GET_DUMPABLE:
      // Core dumps (and debugging) are always possible on the Hurd.
      result = 1;
      break;
    case PR_SET_DUMPABLE:
      // There is no per-process dumpable flag to change; accept the request
      // so callers that toggle it (e.g. memory instrumentation) keep working.
      result = 0;
      break;
    default:
      errno = EINVAL;
      break;
  }
  va_end(ap);
  return result;
}

#endif  // BUILD_CONFIG_HURD_COMPAT_SYS_PRCTL_H_
