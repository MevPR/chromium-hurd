// Copyright 2026 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUILD_CONFIG_HURD_COMPAT_LINUX_RTNETLINK_H_
#define BUILD_CONFIG_HURD_COMPAT_LINUX_RTNETLINK_H_

// Routing netlink data structures for GNU/Hurd builds; see netlink.h.

#include <linux/if_addr.h>
#include <linux/netlink.h>
#include <stdint.h>

enum {
  RTM_NEWLINK = 16,
  RTM_DELLINK = 17,
  RTM_GETLINK = 18,
  RTM_NEWADDR = 20,
  RTM_DELADDR = 21,
  RTM_GETADDR = 22,
  RTM_NEWROUTE = 24,
  RTM_DELROUTE = 25,
  RTM_GETROUTE = 26,
};

struct rtattr {
  unsigned short rta_len;
  unsigned short rta_type;
};

#define RTA_ALIGNTO 4U
#define RTA_ALIGN(len) (((len) + RTA_ALIGNTO - 1) & ~(RTA_ALIGNTO - 1))
#define RTA_OK(rta, len)                                  \
  ((len) >= (int)sizeof(struct rtattr) &&                 \
   (rta)->rta_len >= sizeof(struct rtattr) &&             \
   (rta)->rta_len <= (len))
#define RTA_NEXT(rta, attrlen)                           \
  ((attrlen) -= RTA_ALIGN((rta)->rta_len),               \
   (struct rtattr*)(((char*)(rta)) + RTA_ALIGN((rta)->rta_len)))
#define RTA_LENGTH(len) (RTA_ALIGN(sizeof(struct rtattr)) + (len))
#define RTA_SPACE(len) RTA_ALIGN(RTA_LENGTH(len))
#define RTA_DATA(rta) ((void*)(((char*)(rta)) + RTA_LENGTH(0)))
#define RTA_PAYLOAD(rta) ((int)((rta)->rta_len) - RTA_LENGTH(0))

struct rtgenmsg {
  unsigned char rtgen_family;
};

struct ifinfomsg {
  unsigned char ifi_family;
  unsigned char __ifi_pad;
  unsigned short ifi_type;
  int ifi_index;
  unsigned ifi_flags;
  unsigned ifi_change;
};

#define IFLA_WIRELESS 11
#define IFLA_RTA(r) \
  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))
#define IFLA_PAYLOAD(n) NLMSG_PAYLOAD(n, sizeof(struct ifinfomsg))

#define RTMGRP_LINK 1
#define RTMGRP_NOTIFY 2
#define RTMGRP_IPV4_IFADDR 0x10
#define RTMGRP_IPV6_IFADDR 0x100

#endif  // BUILD_CONFIG_HURD_COMPAT_LINUX_RTNETLINK_H_
