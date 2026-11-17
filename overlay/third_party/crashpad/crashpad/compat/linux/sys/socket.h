// Copyright 2026 The Crashpad Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CRASHPAD_COMPAT_LINUX_SYS_SOCKET_H_
#define CRASHPAD_COMPAT_LINUX_SYS_SOCKET_H_

#include_next <sys/socket.h>

#if defined(__GNU__)
#include <sys/types.h>

// GNU/Hurd has no SO_PASSCRED/SCM_CREDENTIALS. Enabling the option fails at
// runtime, so credentials are never received; these definitions only let the
// Linux handler protocol compile. The SCM value is chosen not to collide with
// the Hurd's own SCM_* types.
struct ucred {
  pid_t pid;
  uid_t uid;
  gid_t gid;
};
#define SO_PASSCRED 16
#define SCM_CREDENTIALS 0x7f02
#endif  // defined(__GNU__)

#endif  // CRASHPAD_COMPAT_LINUX_SYS_SOCKET_H_
