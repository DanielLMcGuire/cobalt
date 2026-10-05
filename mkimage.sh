#!/bin/sh
set -e

EFI=${1:-out/BOOTX64.EFI}
IMG=${2:-out/disk.img}
DISK_SIZE_MB=${SIZE_MB:-256}

echo "Creating ${DISK_SIZE_MB}MB disk image (with persistent partition)"

TOTAL_BYTES=0
[ -f "$EFI" ] && TOTAL_BYTES=$(( TOTAL_BYTES + $(wc -c < "$EFI") ))

if [ -n "$EXTRA" ]; then
    for item in $EXTRA; do
        SRC="${item%%:*}"
        [ -f "$SRC" ] && TOTAL_BYTES=$(( TOTAL_BYTES + $(wc -c < "$SRC") ))
    done
fi

CONTENT_MB=$(( (TOTAL_BYTES + 1048575) / 1048576 ))

ESP_SIZE_MB=$(( CONTENT_MB + 32 ))
[ "$ESP_SIZE_MB" -lt 64 ] && ESP_SIZE_MB=64

PERSIST_DIR=${PERSIST_DIR:-}
if [ -n "$PERSIST_DIR" ]; then
    if [ ! -d "$PERSIST_DIR" ]; then
        echo "error: PERSIST_DIR=$PERSIST_DIR is not a directory" >&2
        exit 1
    fi
    PDIR_MB=$(( ($(du -sk "$PERSIST_DIR" | cut -f1) + 1023) / 1024 ))
    NEED_MB=$(( ESP_SIZE_MB + 2 + PDIR_MB + PDIR_MB / 4 + 8 ))
    if [ "$NEED_MB" -gt "$DISK_SIZE_MB" ]; then
        if [ -n "$SIZE_MB" ]; then
            echo "error: PERSIST_DIR needs ${NEED_MB}MB disk but SIZE_MB=${SIZE_MB}" >&2
            exit 1
        fi
        DISK_SIZE_MB=$NEED_MB
        echo "Growing disk to ${DISK_SIZE_MB}MB to fit PERSIST_DIR (${PDIR_MB}MB of files)"
    fi
fi

if [ "$ESP_SIZE_MB" -ge "$DISK_SIZE_MB" ]; then
    echo "error: Boot files (${ESP_SIZE_MB}MB) exceed target disk size (${DISK_SIZE_MB}MB)" >&2
    exit 1
fi

PERSIST_SIZE_MB=$(( DISK_SIZE_MB - ESP_SIZE_MB - 2 ))
echo "Partition layout: ESP=${ESP_SIZE_MB}MB, Persistent=${PERSIST_SIZE_MB}MB"

dd if=/dev/zero of="$IMG" bs=1M count="$DISK_SIZE_MB" status=none

ESP_END_SECTOR=$(( ESP_SIZE_MB * 2048 + 2048 - 1 ))
PART2_START_SECTOR=$(( ESP_END_SECTOR + 1 ))

sgdisk -o \
    -n "1:2048:${ESP_END_SECTOR}" -t 1:EF00 -c 1:"EFI System Partition" \
    -n "2:${PART2_START_SECTOR}:0" -t 2:8300 -c 2:"PERSIST" \
    "$IMG" >/dev/null

mformat -i "$IMG@@1M" -F -h 32 -t 32 -n 64 ::
mmd -i "$IMG@@1M" ::/EFI ::/EFI/BOOT
mcopy -i "$IMG@@1M" "$EFI" ::/EFI/BOOT/BOOTX64.EFI

if [ -n "$EXTRA" ]; then
    for item in $EXTRA; do
        SRC="${item%%:*}"
        DST="${item##*:}"
        if [ -f "$SRC" ]; then
            mcopy -i "$IMG@@1M" "$SRC" "::/$DST"
        fi
    done
fi

PART2_OFFSET_BYTES=$(( PART2_START_SECTOR * 512 ))
PART2_BLOCKS=$(( PERSIST_SIZE_MB * 1024 ))

set -- -t ext4 -F -q -L "PERSIST" -E "offset=$PART2_OFFSET_BYTES,root_owner=0:0"
if [ -n "$PERSIST_DIR" ]; then
    echo "Populating persistent partition from $PERSIST_DIR"
    set -- "$@" -d "$PERSIST_DIR"
fi
mke2fs "$@" "$IMG" "$PART2_BLOCKS"

echo "wrote $IMG (${DISK_SIZE_MB}MB with persistent partition)"