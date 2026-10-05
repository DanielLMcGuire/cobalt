#!/bin/bash
set -euo pipefail

sed -i 's#https://gmplib.org/download/gmp/#https://ftp.gnu.org/gnu/gmp/#g' PKGBUILD
grep -q 'ftp.gnu.org/gnu/gmp/' PKGBUILD \
    || { echo "gmp hook: could not find the gmplib.org source URL in the PKGBUILD (layout changed?)" >&2; exit 1; }
echo "  gmp hook: source host gmplib.org -> ftp.gnu.org"