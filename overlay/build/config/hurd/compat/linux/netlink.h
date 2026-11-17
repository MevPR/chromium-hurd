// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUILD_CONFIG_HURD_COMPAT_LINUX_NETLINK_H_
#define BUILD_CONFIG_HURD_COMPAT_LINUX_NETLINK_H_

// Netlink data structures for GNU/Hurd builds.
//
// The Hurd has no netlink. Chromium's network change tracking
// (net/base/address_tracker_linux.cc) uses these types in its interfaces and
// handles the failure of socket(AF_NETLINK, ...) at runtime by reporting the
// network as online. Only the definitions Chromium uses are provided; the
// values match the Linux UAPI so the code behaves identically.

#include <linux/types.h>
#include <stdint.h>
#include <sys/socket.h>

#ifndef AF_NETLINK
#define AF_NETLINK 16
#endif
#ifndef PF_NETLINK
#define PF_NETLINK AF_NETLINK
#endif

#define NETLINK_ROUTE 0

struct sockaddr_nl {
  sa_family_t nl_family;
  unsigned short nl_pad;
  uint32_t nl_pid;
  uint32_t nl_groups;
};

struct nlmsghdr {
  uint32_t nlmsg_len;
  uint16_t nlmsg_type;
  uint16_t nlmsg_flags;
  uint32_t nlmsg_seq;
  uint32_t nlmsg_pid;
};

#define NLM_F_REQUEST 0x01
#define NLM_F_MULTI 0x02
#define NLM_F_ROOT 0x100
#define NLM_F_MATCH 0x200
#define NLM_F_DUMP (NLM_F_ROOT | NLM_F_MATCH)

#define NLMSG_ALIGNTO 4U
#define NLMSG_ALIGN(len) (((len) + NLMSG_ALIGNTO - 1) & ~(NLMSG_ALIGNTO - 1))
#define NLMSG_HDRLEN ((int)NLMSG_ALIGN(sizeof(struct nlmsghdr)))
#define NLMSG_LENGTH(len) ((len) + NLMSG_HDRLEN)
#define NLMSG_SPACE(len) NLMSG_ALIGN(NLMSG_LENGTH(len))
#define NLMSG_DATA(nlh) ((void*)(((char*)nlh) + NLMSG_HDRLEN))
#define NLMSG_NEXT(nlh, len)                     \
  ((len) -= NLMSG_ALIGN((nlh)->nlmsg_len),       \
   (struct nlmsghdr*)(((char*)(nlh)) +           \
                      NLMSG_ALIGN((nlh)->nlmsg_len)))
#define NLMSG_OK(nlh, len)                                \
  ((len) >= (int)sizeof(struct nlmsghdr) &&               \
   (nlh)->nlmsg_len >= sizeof(struct nlmsghdr) &&         \
   (nlh)->nlmsg_len <= (len))
#define NLMSG_PAYLOAD(nlh, len) ((nlh)->nlmsg_len - NLMSG_SPACE((len)))

#define NLMSG_NOOP 0x1
#define NLMSG_ERROR 0x2
#define NLMSG_DONE 0x3

struct nlmsgerr {
  int error;
  struct nlmsghdr msg;
};

#endif  // BUILD_CONFIG_HURD_COMPAT_LINUX_NETLINK_H_
