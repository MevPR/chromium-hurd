// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUILD_CONFIG_HURD_COMPAT_LINUX_SYNC_FILE_H_
#define BUILD_CONFIG_HURD_COMPAT_LINUX_SYNC_FILE_H_

// Sync file (DMA fence) ioctl definitions for GNU/Hurd builds, needed by
// //third_party/libsync. The Hurd has no DRM drivers, so sync file
// descriptors never exist and these ioctls are never issued successfully.

#include <stdint.h>
#include <sys/ioctl.h>

struct sync_merge_data {
  char name[32];
  int32_t fd2;
  int32_t fence;
  uint32_t flags;
  uint32_t pad;
};

struct sync_fence_info {
  char obj_name[32];
  char driver_name[32];
  int32_t status;
  uint32_t flags;
  uint64_t timestamp_ns;
};

struct sync_file_info {
  char name[32];
  int32_t status;
  uint32_t flags;
  uint32_t num_fences;
  uint32_t pad;
  uint64_t sync_fence_info;
};

#define SYNC_IOC_MAGIC '>'
#define SYNC_IOC_MERGE _IOWR(SYNC_IOC_MAGIC, 3, struct sync_merge_data)
#define SYNC_IOC_FILE_INFO _IOWR(SYNC_IOC_MAGIC, 4, struct sync_file_info)

#endif  // BUILD_CONFIG_HURD_COMPAT_LINUX_SYNC_FILE_H_
