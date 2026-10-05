#!/bin/bash

set -euo pipefail

KEEP=(gcc gcc-libs libgcc 'libstdc++')

sed -i -E '/--enable-languages="?jit"?([[:space:]\\]|$)/! s/(--enable-languages=)"?[^[:space:]\\"]+"?/\1c,c++,lto/' PKGBUILD
grep -q 'enable-languages=c,c++,lto' PKGBUILD \
    || { echo "gcc hook: could not find --enable-languages in the PKGBUILD" >&2; exit 1; }

mapfile -t all < <(makepkg --printsrcinfo | sed -n 's/^pkgname = //p')
[ "${#all[@]}" -gt 0 ] || { echo "gcc hook: could not read package names from the PKGBUILD" >&2; exit 1; }
keep=()
for n in "${all[@]}"; do
    for k in "${KEEP[@]}"; do [ "$n" = "$k" ] && keep+=("$n"); done
done
[ "${#keep[@]}" -gt 0 ] || { echo "gcc hook: none of KEEP exist in this recipe" >&2; exit 1; }

sed -i '/^# gcc-hook-begin/,/^# gcc-hook-end/d' PKGBUILD
{
    echo '# gcc-hook-begin'
    printf 'pkgname=('; printf '"%s" ' "${keep[@]}"; echo ')'
    [ "${#keep[@]}" -eq 1 ] && echo "if ! declare -f package >/dev/null; then package() { package_${keep[0]}; }; fi"
    cat <<'EOS'
mv() {
    local -a opts=() srcs=() ok=()
    local a s dest
    for a in "$@"; do
        case "$a" in
            -t|--target-directory*|-T) command mv "$@"; return ;;
            -*) opts+=("$a") ;;
            *)  srcs+=("$a") ;;
        esac
    done
    if [ "${#srcs[@]}" -lt 2 ]; then command mv "$@"; return; fi
    dest=${srcs[-1]}; unset 'srcs[-1]'
    for s in "${srcs[@]}"; do
        if [ -e "$s" ] || [ -L "$s" ]; then ok+=("$s"); fi
    done
    if [ "${#ok[@]}" -eq 0 ]; then
        echo "mv: (gcc hook) skipping, language not built: ${srcs[*]}" >&2
        return 0
    fi
    command mv "${opts[@]}" "${ok[@]}" "$dest"
}
EOS
    echo '# gcc-hook-end'
} >> PKGBUILD
echo "  gcc hook: C/C++/LTO only; building: ${keep[*]}"
