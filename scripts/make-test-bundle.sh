#!/bin/sh
# Packs a self-contained test bundle of the Hurd build: stripped chrome, its
# resources, and every runtime library of the sysroot (including glibc and
# the dynamic linker). This lets the build run on a Debian GNU/Hurd system
# whose installed libraries are older than the sysroot, without touching
# them. Run inside the build container.
#
# Usage: make-test-bundle.sh SRCDIR SYSROOT OUTPUT.tar.gz
# On the Hurd: mkdir /opt/chromium && tar xzf OUTPUT.tar.gz -C /opt/chromium
#              /opt/chromium/run-chrome --version
set -eu
SRC=${1:?usage: make-test-bundle.sh SRCDIR SYSROOT OUTPUT.tar.gz}
SYSROOT=$(cd "${2:?missing SYSROOT}" && pwd)
OUT=$(realpath "${3:?missing OUTPUT}")
BUILD=$SRC/out/hurd
STRIP=$SRC/third_party/llvm-build/Release+Asserts/bin/llvm-strip

STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT
mkdir -p "$STAGE/lib" "$STAGE/locales"

"$STRIP" --strip-debug -o "$STAGE/chrome" "$BUILD/chrome"
for f in "$BUILD"/*.pak "$BUILD"/icudtl.dat "$BUILD"/*.bin \
         "$BUILD"/libEGL.so "$BUILD"/libGLESv2.so "$BUILD"/libvk_swiftshader.so \
         "$BUILD"/vk_swiftshader_icd.json; do
  [ -e "$f" ] || continue
  case "$f" in
    *.so) "$STRIP" --strip-debug -o "$STAGE/$(basename "$f")" "$f" ;;
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
# Starts the bundled chrome with the bundled dynamic linker and libraries.
HERE=$(cd "$(dirname "$0")" && pwd)
exec "$HERE/lib/ld-x86-64.so.1" --library-path "$HERE/lib:$HERE" \
  "$HERE/chrome" --no-sandbox --no-zygote "$@"
EOF
chmod +x "$STAGE/run-chrome"

tar czf "$OUT" -C "$STAGE" .
ls -la "$OUT"
