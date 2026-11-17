// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_EPOLL_SHIM_HURD_H_
#define BASE_MESSAGE_LOOP_EPOLL_SHIM_HURD_H_

// GNU/Hurd has no epoll. MessagePumpEpoll always takes its poll() code path
// there, so the epoll calls it makes for bookkeeping only need to succeed.
// This header provides the epoll types and constants plus no-op functions.

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>

#include "build/build_config.h"

static_assert(BUILDFLAG(IS_HURD), "Only for GNU/Hurd");

#ifndef POLLRDHUP
#define POLLRDHUP 0
#endif

#define EPOLLIN 0x001u
#define EPOLLPRI 0x002u
#define EPOLLOUT 0x004u
#define EPOLLERR 0x008u
#define EPOLLHUP 0x010u
#define EPOLLRDHUP 0x2000u
#define EPOLLONESHOT (1u << 30)

#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3

typedef union epoll_data {
  void* ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

struct epoll_event {
  uint32_t events;
  epoll_data_t data;
};

// Returns a valid descriptor so callers that check it keep working.
inline int epoll_create1(int) {
  return open("/dev/null", O_RDONLY | O_CLOEXEC);
}

inline int epoll_ctl(int, int, int, struct epoll_event*) {
  return 0;
}

inline int epoll_wait(int, struct epoll_event*, int, int) {
  errno = ENOSYS;
  return -1;
}

#endif  // BASE_MESSAGE_LOOP_EPOLL_SHIM_HURD_H_
