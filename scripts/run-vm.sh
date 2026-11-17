#!/bin/sh
# Boots the Debian GNU/Hurd amd64 image in QEMU for testing.
# Usage: run-vm.sh IMAGE [extra qemu args]
# SSH is forwarded to 127.0.0.1:2222, VNC to 127.0.0.1:5905.
set -eu
IMG=${1:?usage: run-vm.sh IMAGE}
shift
ACCEL=tcg
if [ "$(uname -s)" = Linux ] && [ "$(uname -m)" = x86_64 ] && [ -w /dev/kvm ]; then
  ACCEL=kvm
fi
# AHCI instead of the default PIIX IDE: the Hurd's rumpdisk reports I/O errors
# on QEMU's emulated IDE controller when running under TCG.
exec qemu-system-x86_64 -accel "$ACCEL" -m 4G \
  -device ahci,id=ahci \
  -drive id=disk,file="$IMG",if=none,format=raw,cache=writeback \
  -device ide-hd,drive=disk,bus=ahci.0 \
  -nic user,hostfwd=tcp:127.0.0.1:2222-:22 \
  -display none -vnc 127.0.0.1:5 "$@"
