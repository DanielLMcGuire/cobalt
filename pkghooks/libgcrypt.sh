#!/bin/bash
set -euo pipefail

KEY=AC8E115BF73E2D8D47FA9908E98E9B2D19C6C8BD

listed=$(bash -c 'source ./PKGBUILD >/dev/null 2>&1; printf "%s\n" "${validpgpkeys[@]}"' || true)
if grep -qx "$KEY" <<<"$listed"; then
    echo "  libgcrypt hook: $KEY already in validpgpkeys"
    exit 0
fi

printf '\nvalidpgpkeys+=("%s") # Niibe Yutaka (GnuPG Release Key)\n' "$KEY" >> PKGBUILD
echo "  libgcrypt hook: added $KEY to validpgpkeys"
