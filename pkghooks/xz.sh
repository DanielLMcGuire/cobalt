#!/bin/bash

set -euo pipefail

if grep -q 'no-po4a' PKGBUILD; then
    echo "  xz hook: PKGBUILD already passes --no-po4a"
    exit 0
fi
sed -i -E 's/(autogen\.sh)([[:space:]]|$)/\1 --no-po4a\2/' PKGBUILD
grep -q 'autogen.sh --no-po4a' PKGBUILD \
    || { echo "xz hook: no autogen.sh call found in the PKGBUILD (layout changed?)" >&2; exit 1; }
echo "  xz hook: autogen.sh now runs with --no-po4a (translated man pages skipped)"
