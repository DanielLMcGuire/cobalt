#!/bin/sh
set -e
OUT=${1:-out/initrd-offline.img}
MODULES_PATH=${MODULES_PATH:-$2}
BB=${BUSYBOX:-}
[ -n "$BB" ] || BB=$(command -v busybox || true)
if [ -z "$BB" ] && [ -n "$OVERLAY" ] && [ -x "$OVERLAY/usr/bin/busybox" ]; then
    BB="$OVERLAY/usr/bin/busybox"
fi

if [ -z "$BB" ] || [ ! -x "$BB" ]; then
    echo "error: busybox not found (build one with buildk.sh, or set BUSYBOX=/path/to/busybox)" >&2
    exit 1
fi

bb_run() {
    if "$BB" --list >/dev/null 2>&1; then
        "$BB" "$@"
    else
        _ld="$OVERLAY/usr/lib/ld-linux-x86-64.so.2"
        [ -x "$_ld" ] || { echo "error: cannot execute $BB on this host and no overlay loader found" >&2; exit 1; }
        "$_ld" --library-path "$OVERLAY/usr/lib" "$BB" "$@"
    fi
}

ROOT=$(mktemp -d)

cleanup() { chmod -R u+w "$ROOT" 2>/dev/null || true; rm -rf "$ROOT" || true; }
trap cleanup EXIT

if [ -n "$OVERLAY" ]; then
    if [ ! -d "$OVERLAY" ]; then
        echo "error: OVERLAY=$OVERLAY is not a directory" >&2
        exit 1
    fi
    cp -a "$OVERLAY"/. "$ROOT"/
    chmod -R u+w "$ROOT" 2>/dev/null || true
fi

mkdir -p "$ROOT"/bin "$ROOT"/sbin "$ROOT"/usr/bin "$ROOT"/usr/sbin \
         "$ROOT"/etc/profile.d "$ROOT"/proc "$ROOT"/sys "$ROOT"/dev "$ROOT"/tmp \
         "$ROOT"/root "$ROOT"/mnt "$ROOT"/run "$ROOT"/var "$ROOT"/lib/modules \
         "$ROOT"/persist

cp "$BB" "$ROOT/bin/busybox"
chmod 755 "$ROOT/bin/busybox"

for a in $(bb_run --list); do
    [ "$a" = busybox ] && continue
    if [ -e "$ROOT/bin/$a" ] || [ -L "$ROOT/bin/$a" ]; then continue; fi
    ln -s busybox "$ROOT/bin/$a"
done

if [ ! -e "$ROOT/init" ] && [ ! -L "$ROOT/init" ]; then
    ln -s bin/busybox "$ROOT/init"
fi

if [ ! -e "$ROOT/sbin/init" ] && [ ! -L "$ROOT/sbin/init" ]; then
    ln -s ../bin/busybox "$ROOT/sbin/init"
fi

cat > "$ROOT/etc/rc" <<'EOF'
#!/bin/sh
export PATH=/bin:/sbin:/usr/bin:/usr/sbin

mkdir -p /proc /sys /dev /tmp /run
mount -t proc     proc     /proc
mount -t sysfs    sysfs    /sys
mount -t devtmpfs devtmpfs /dev

mkdir -p /dev/pts /dev/shm
mount -t devpts   devpts   /dev/pts
mount -t tmpfs    tmpfs    /dev/shm
mount -t tmpfs    tmpfs    /tmp
mount -t tmpfs    tmpfs    /run

mdev -s 2>/dev/null || true
{ echo /sbin/mdev > /proc/sys/kernel/hotplug; } 2>/dev/null || true

if [ -d /lib/modules ]; then
    modprobe xhci_pci 2>/dev/null || true
    modprobe xhci_hcd 2>/dev/null || true
    modprobe ehci_pci 2>/dev/null || true
    modprobe ehci_hcd 2>/dev/null || true
    modprobe uhci_hcd 2>/dev/null || true
    modprobe ohci_pci 2>/dev/null || true
    modprobe ohci_hcd 2>/dev/null || true

    modprobe usbcore 2>/dev/null || true
    modprobe hid 2>/dev/null || true
    modprobe hid_generic 2>/dev/null || true
    modprobe usbhid 2>/dev/null || true
    modprobe hid_apple 2>/dev/null || true

    modprobe i8042 2>/dev/null || true
    modprobe atkbd 2>/dev/null || true
    modprobe serio 2>/dev/null || true
    modprobe serio_raw 2>/dev/null || true

    modprobe intel_lpss_pci 2>/dev/null || true
    modprobe i2c_hid 2>/dev/null || true
    modprobe i2c_hid_acpi 2>/dev/null || true

    if grep -qi 'microsoft' /sys/class/dmi/id/sys_vendor 2>/dev/null; then
        modprobe hv_vmbus        2>/dev/null || true
        modprobe hyperv_keyboard 2>/dev/null || true
        modprobe hid_hyperv      2>/dev/null || true
        modprobe hv_storvsc      2>/dev/null || true
    fi

    modprobe evdev 2>/dev/null || true
    modprobe usb_storage 2>/dev/null || true
    modprobe uas 2>/dev/null || true
    modprobe vfat 2>/dev/null || true
    modprobe nls_cp437 2>/dev/null || true
    modprobe nls_iso8859_1 2>/dev/null || true
fi

modprobe libata     2>/dev/null || true
modprobe ahci       2>/dev/null || true
modprobe ata_piix   2>/dev/null || true
modprobe sd_mod     2>/dev/null || true
modprobe nvme       2>/dev/null || true
modprobe virtio_blk 2>/dev/null || true

mkdir -p /persist
i=0
while [ $i -lt 20 ]; do
    mdev -s 2>/dev/null
    for p in /dev/sd?2 /dev/vd?2 /dev/nvme?n?p2; do
        [ -b "$p" ] || continue
        if mount -t ext4 "$p" /persist 2>/dev/null; then
            echo "persist: mounted $p at /persist"
            break 2
        fi
    done
    i=$((i+1)); sleep 0.25
done
grep -q ' /persist ' /proc/mounts || echo "warning: PERSIST partition not mounted"

if command -v ldconfig >/dev/null 2>&1; then
    ldconfig 2>/dev/null || true
elif [ -x /usr/sbin/ldconfig.real ]; then
    /usr/sbin/ldconfig.real 2>/dev/null || true
elif [ -x /sbin/ldconfig ]; then
    /sbin/ldconfig 2>/dev/null || true
fi

hostname cobalt
EOF
chmod 755 "$ROOT/etc/rc"

cat > "$ROOT/etc/inittab" <<'EOF'
::sysinit:/etc/rc
tty1::respawn:/bin/sh -c 'ENV=/etc/profile exec /bin/sh'
ttyS0::respawn:/bin/sh -c 'ENV=/etc/profile exec /bin/sh'
::ctrlaltdel:/sbin/reboot
EOF

cat > "$ROOT/etc/profile.d/99-motd.sh" <<'EOF'
echo " Welcome to Cobalt Linux!"
echo " Type 'help' for built-in commands."
echo ""
EOF
chmod 755 "$ROOT/etc/profile.d/99-motd.sh"

cat > "$ROOT/etc/profile" <<'EOF'
export PATH=/bin:/sbin:/usr/bin:/usr/sbin
export HOME=/root
export PS1='\u@\h:\w\$ '

for f in /etc/profile.d/*.sh; do
    [ -f "$f" ] && . "$f"
done
EOF

echo 'root:x:0:0:root:/root:/bin/sh' > "$ROOT/etc/passwd"
echo 'root:x:0:'                     > "$ROOT/etc/group"
echo 'cobalt'                       > "$ROOT/etc/hostname"

if [ -n "$MODULES_PATH" ] && [ -d "$MODULES_PATH" ]; then
    if [ -f "$MODULES_PATH/version.txt" ]; then
        KVER=$(tr -d '\r\n' < "$MODULES_PATH/version.txt")
    elif [ -f "$MODULES_PATH/../version.txt" ]; then
        KVER=$(tr -d '\r\n' < "$MODULES_PATH/../version.txt")
    else
        KVER=$(basename "$MODULES_PATH")
    fi

    echo "Extracting modules for version: $KVER"
    DST_MOD="$ROOT/lib/modules/$KVER"
    mkdir -p "$DST_MOD"

    for meta in modules.order modules.builtin modules.builtin.modinfo; do
        if [ -f "$MODULES_PATH/$meta" ]; then
            cp "$MODULES_PATH/$meta" "$DST_MOD/"
        fi
    done

    for sub in drivers/usb drivers/hid drivers/input drivers/i2c drivers/mfd \
               drivers/ata drivers/scsi drivers/nvme drivers/block drivers/hv \
               fs/fat fs/nls; do
        if [ -d "$MODULES_PATH/kernel/$sub" ]; then
            mkdir -p "$DST_MOD/kernel/$sub"
            cp -a "$MODULES_PATH/kernel/$sub"/. "$DST_MOD/kernel/$sub/"
        fi
    done

    if find "$DST_MOD" -name "*.ko.zst" -print -quit | grep -q .; then
        echo "Decompressing modules for BusyBox..."
        if command -v zstd >/dev/null 2>&1; then
            find "$DST_MOD" -name "*.ko.zst" -exec zstd -d --rm -q {} +
        elif command -v unzstd >/dev/null 2>&1; then
            find "$DST_MOD" -name "*.ko.zst" -exec unzstd --rm -q {} +
        fi
    fi

    if command -v depmod >/dev/null 2>&1; then
        echo "Rebuilding modules.dep with depmod..."
        depmod -a -b "$ROOT" "$KVER"
    fi
fi

if [ -x "$ROOT/bin/bash" ] || [ -x "$ROOT/usr/bin/bash" ]; then
    sed -i 's|/bin/sh.*|/bin/bash --login|g' "$ROOT/etc/inittab"
    sed -i 's|/bin/sh|/bin/bash|g' "$ROOT/etc/passwd"
fi

mknod -m 600 "$ROOT/dev/console" c 5 1 2>/dev/null || true
mknod -m 666 "$ROOT/dev/null"    c 1 3 2>/dev/null || true

UNCOMPRESSED_KB=$(du -sk "$ROOT" | cut -f1)

mkdir -p "$(dirname "$OUT")"
( cd "$ROOT" && find . | sort | cpio -o -H newc -R 0:0 --quiet | gzip -9 ) > "$OUT"

COMPRESSED_BYTES=$(wc -c < "$OUT")
COMPRESSED_MB=$(( (COMPRESSED_BYTES + 1048575) / 1048576 ))

echo "wrote $OUT: ${COMPRESSED_MB}MB compressed (uncompressed RAM footprint: $(( UNCOMPRESSED_KB / 1024 ))MB)"