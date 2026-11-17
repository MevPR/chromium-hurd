# GNU/Hurd compatibility headers

This directory is added with `-isystem` to every compilation for
`target_os = "hurd"` (see `//build/config/compiler:compiler`).

It contains small replacements for Linux-only headers that are used in many
places across Chromium and third-party code, where patching every user would
be impractical:

* `sys/prctl.h`: `prctl()` supporting `PR_SET_NAME` / `PR_GET_NAME`
  (mapped to `pthread_{set,get}name_np()`); other options fail with `EINVAL`.

Keep this list short. Prefer fixing the user when there are only a few.
