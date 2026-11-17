// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUILD_CONFIG_HURD_COMPAT_LINUX_SYNC_FILE_H_
#define BUILD_CONFIG_HURD_COMPAT_LINUX_SYNC_FILE_H_

// Sync file (DMA fence) ioctl definitions for GNU/Hurd builds, needed by
// //third_party/libsync's headers. The Hurd has no DRM drivers, so sync file
// descriptors never exist (libsync is replaced by sync_hurd.c).

#include <stdint.h>

// <linux/types.h> fixed-width types, as used by sync file users.
typedef int8_t __s8;
typedef uint8_t __u8;
typedef int16_t __s16;
typedef uint16_t __u16;
typedef int32_t __s32;
typedef uint32_t __u32;
typedef int64_t __s64;
typedef uint64_t __u64;

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

// The Hurd's _IOWR() needs a type description for each struct, so encode the
// numbers as Linux does. They are never issued successfully on the Hurd.
#define SYNC_IOC_MAGIC '>'
#define __SYNC_HURD_IOWR(nr, size) \
  ((unsigned long)((3U << 30) | ((size) << 16) | (SYNC_IOC_MAGIC << 8) | (nr)))
#define SYNC_IOC_MERGE __SYNC_HURD_IOWR(3, sizeof(struct sync_merge_data))
#define SYNC_IOC_FILE_INFO __SYNC_HURD_IOWR(4, sizeof(struct sync_file_info))

#endif  // BUILD_CONFIG_HURD_COMPAT_LINUX_SYNC_FILE_H_
