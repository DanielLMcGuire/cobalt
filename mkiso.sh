#!/bin/sh
set -e

IMG=${1:-out/disk.img}
ISO=${2:-out/uefi-linux.iso}

T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
mkdir -p "$T/files" "$T/iso"

mcopy -s -n -i "$IMG@@1M" :: "$T/files/"

SIZE_KB=$(du -sk "$T/files" | cut -f1)
FAT_MB=$(( SIZE_KB / 1024 + 16 ))
dd if=/dev/zero of="$T/iso/efiboot.img" bs=1M count="$FAT_MB" status=none
mformat -i "$T/iso/efiboot.img" -F -T $(( FAT_MB * 2048 )) ::
mcopy -s -i "$T/iso/efiboot.img" "$T/files"/* ::/

mkdir -p "$(dirname "$ISO")"
xorriso -as mkisofs -iso-level 3 -V UEFI_LINUX \
    -o "$ISO" \
    -e efiboot.img -no-emul-boot \
    -isohybrid-gpt-basdat \
    "$T/iso"

echo "wrote $ISO ($(( $(wc -c < "$ISO") / 1048576 ))MB)"