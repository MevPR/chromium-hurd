// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// libsync for GNU/Hurd. The Hurd has no DRM drivers and therefore no sync
// files (DMA fences). sync_wait() works on any pollable descriptor, as the
// original implementation does; the sync file ioctls fail with ENOSYS.

#include <errno.h>
#include <poll.h>
#include <stddef.h>
#include <stdint.h>

#include "ndk/sync.h"

int sync_wait(int fd, int timeout) {
  struct pollfd fds;
  fds.fd = fd;
  fds.events = POLLIN;
  int ret;
  do {
    ret = poll(&fds, 1, timeout);
    if (ret > 0) {
      if (fds.revents & (POLLERR | POLLNVAL)) {
        errno = EINVAL;
        return -1;
      }
      return 0;
    }
    if (ret == 0) {
      errno = ETIME;
      return -1;
    }
  } while (ret == -1 && (errno == EINTR || errno == EAGAIN));
  return ret;
}

int32_t sync_merge(const char* name, int32_t fd1, int32_t fd2) {
  (void)name;
  (void)fd1;
  (void)fd2;
  errno = ENOSYS;
  return -1;
}

struct sync_file_info* sync_file_info(int32_t fd) {
  (void)fd;
  errno = ENOSYS;
  return NULL;
}

void sync_file_info_free(struct sync_file_info* info) {
  (void)info;
}
