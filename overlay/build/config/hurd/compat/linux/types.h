// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUILD_CONFIG_HURD_COMPAT_LINUX_TYPES_H_
#define BUILD_CONFIG_HURD_COMPAT_LINUX_TYPES_H_

// The fixed-width integer types of <linux/types.h>, used by code that
// describes Linux kernel data structures (netlink, DMA fences, seccomp
// policies) on GNU/Hurd builds.

#include <stddef.h>
#include <stdint.h>

typedef int8_t __s8;
typedef uint8_t __u8;
typedef int16_t __s16;
typedef uint16_t __u16;
typedef int32_t __s32;
typedef uint32_t __u32;
typedef int64_t __s64;
typedef uint64_t __u64;
typedef __u16 __le16;
typedef __u16 __be16;
typedef __u32 __le32;
typedef __u32 __be32;
typedef __u64 __le64;
typedef __u64 __be64;
typedef size_t __kernel_size_t;

#endif  // BUILD_CONFIG_HURD_COMPAT_LINUX_TYPES_H_
