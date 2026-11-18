#!/bin/sh
# Packs a self-contained test bundle of the Hurd build: stripped chrome, its
# resources, and every runtime library of the sysroot (including glibc and
# the dynamic linker). This lets the build run on a Debian GNU/Hurd system
# whose installed libraries are older than the sysroot, without touching
# them. Run inside the build container.
#
# Usage: make-test-bundle.sh SRCDIR SYSROOT OUTPUT.tar.gz [PREFIX]
# On the Hurd: mkdir -p /opt/chromium && tar xzf OUTPUT.tar.gz -C /opt/chromium
#              /opt/chromium/run-chrome --version
#
# chrome's ELF interpreter and RPATH point into PREFIX (default
# /opt/chromium), so the bundle must be unpacked there. Starting it through
# the bundled ld.so instead would make /proc/self/exe point at ld.so, which
# breaks resource lookup and the relaunching of child processes.
set -eu
SRC=${1:?usage: make-test-bundle.sh SRCDIR SYSROOT OUTPUT.tar.gz}
SYSROOT=$(cd "${2:?missing SYSROOT}" && pwd)
OUT=$(realpath "${3:?missing OUTPUT}")
PREFIX=${4:-/opt/chromium}
BUILD=$SRC/out/hurd
STRIP=$SRC/third_party/llvm-build/Release+Asserts/bin/llvm-strip

STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT
mkdir -p "$STAGE/lib" "$STAGE/locales"

"$STRIP" --strip-all -o "$STAGE/chrome" "$BUILD/chrome"
patchelf --set-interpreter "$PREFIX/lib/ld-x86-64.so.1" \
  --force-rpath --set-rpath "$PREFIX/lib:$PREFIX" "$STAGE/chrome"
for f in "$BUILD"/*.pak "$BUILD"/icudtl.dat "$BUILD"/*.bin \
         "$BUILD"/libEGL.so "$BUILD"/libGLESv2.so "$BUILD"/libvk_swiftshader.so \
         "$BUILD"/libvulkan.so.1 "$BUILD"/vk_swiftshader_icd.json; do
  [ -e "$f" ] || continue
  case "$f" in
    *.so|*.so.1) "$STRIP" --strip-debug -o "$STAGE/$(basename "$f")" "$f" ;;
    *) cp "$f" "$STAGE/" ;;
  esac
done
cp "$BUILD/locales/en-US.pak" "$STAGE/locales/"

# Runtime libraries, symlinks preserved. Unversioned *.so files are included
# because some libraries (NSPR, NSS) are only installed under that name; the
# dynamic linker, ld-x86-64.so.1, matches the second pattern.
cp -a "$SYSROOT"/usr/lib/x86_64-gnu/*.so "$SYSROOT"/usr/lib/x86_64-gnu/*.so.* \
  "$STAGE/lib/"

cat > "$STAGE/run-chrome" <<'EOF'
#!/bin/sh
# chrome uses the bundled dynamic linker and libraries (ELF interpreter and
# RPATH); the port needs these flags for now. Under QEMU without KVM a child
# process can take longer than the default 15 s to receive the browser's first
# IPC reply and would then terminate itself, so the timeout is raised. For the
# same reason the GPU watchdog would kill the GPU process while SwiftShader
# renders.
HERE=$(cd "$(dirname "$0")" && pwd)
exec "$HERE/chrome" --no-sandbox --no-zygote --ipc-connection-timeout=120 \
  --disable-gpu-watchdog "$@"
EOF
chmod +x "$STAGE/run-chrome"

tar czf "$OUT" -C "$STAGE" .
ls -la "$OUT"
