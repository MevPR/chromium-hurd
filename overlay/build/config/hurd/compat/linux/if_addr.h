// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUILD_CONFIG_HURD_COMPAT_LINUX_IF_ADDR_H_
#define BUILD_CONFIG_HURD_COMPAT_LINUX_IF_ADDR_H_

// Interface address netlink structures for GNU/Hurd builds; see netlink.h.

#include <linux/netlink.h>
#include <stdint.h>

struct ifaddrmsg {
  uint8_t ifa_family;
  uint8_t ifa_prefixlen;
  uint8_t ifa_flags;
  uint8_t ifa_scope;
  uint32_t ifa_index;
};

enum {
  IFA_UNSPEC,
  IFA_ADDRESS,
  IFA_LOCAL,
  IFA_LABEL,
  IFA_BROADCAST,
  IFA_ANYCAST,
  IFA_CACHEINFO,
  IFA_MULTICAST,
  IFA_FLAGS,
};

#define IFA_F_TEMPORARY 0x01
#define IFA_F_OPTIMISTIC 0x04
#define IFA_F_HOMEADDRESS 0x10
#define IFA_F_DADFAILED 0x08
#define IFA_F_DEPRECATED 0x20
#define IFA_F_TENTATIVE 0x40

struct ifa_cacheinfo {
  uint32_t ifa_prefered;
  uint32_t ifa_valid;
  uint32_t cstamp;
  uint32_t tstamp;
};

#define IFA_RTA(r) \
  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg))))
#define IFA_PAYLOAD(n) NLMSG_PAYLOAD(n, sizeof(struct ifaddrmsg))

#endif  // BUILD_CONFIG_HURD_COMPAT_LINUX_IF_ADDR_H_
