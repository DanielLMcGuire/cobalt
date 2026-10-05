#!/bin/bash
set -euo pipefail

if grep -q -- '-std=gnu17' PKGBUILD; then
    echo "  libsasl hook: PKGBUILD already sets -std=gnu17"
    exit 0
fi

printf '\nCFLAGS+=" -std=gnu17"\n' >> PKGBUILD
echo "  libsasl hook: building with -std=gnu17 (GCC >= 15 defaults to C23)"
