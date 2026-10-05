#!/bin/bash
echo "Patching libxcrypt to disable -Werror and strict const checks..."

sed -i 's|/configure|/configure --disable-werror|g' PKGBUILD

sed -i '/^build()/a \  export CFLAGS="$CFLAGS -Wno-error=discarded-qualifiers -Wno-error=incompatible-pointer-types"' PKGBUILD
