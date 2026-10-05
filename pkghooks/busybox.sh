#!/bin/bash
set -euo pipefail

files=$(grep -rlE '^(# )?CONFIG_TC[= ]' . --exclude-dir=.git || true)
if [ -z "$files" ]; then
    echo "  busybox hook: no busybox config found (generated at build time?)"
    exit 0
fi

set_opt() {
    local f=$1 name=$2 val=$3 want
    if [ "$val" = y ]; then want="CONFIG_$name=y"; else want="# CONFIG_$name is not set"; fi
    grep -qxF "$want" "$f" && return 0
    if grep -qE "^(# )?CONFIG_$name([= ]|\$)" "$f"; then
        sed -i -E "s|^(# )?CONFIG_$name([= ].*)?\$|$want|" "$f"
    else
        echo "$want" >> "$f"
    fi
    echo "  busybox hook: $want in $f"
    changed=1
}

changed=0
for f in $files; do
    set_opt "$f" TC n
    set_opt "$f" FEATURE_TC_INGRESS n
    set_opt "$f" FEATURE_INIT_QUIET y
done

[ "$changed" = 1 ] && touch .buildk-refresh-sums
exit 0