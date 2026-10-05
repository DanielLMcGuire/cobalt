CC = clang
CFLAGS = --target=x86_64-unknown-windows -std=gnu2x -O2 \
          -ffreestanding -fno-builtin -fno-stack-protector -mno-stack-arg-probe -mno-red-zone \
          -U_WIN32 -U_WIN64 -D__UEFI__ -D_CORECRT_BUILD \
          -Ixxc/libminicrt/include -Ixxc/libminicrt/internal -Ixxc/libxxc/include -Ixxc/platform/uefi \
          -Wall -Wno-unused-function
OVMF ?= /usr/share/OVMF

KERNEL ?= vmlinuz

QEMU_FLAGS ?=

PERSIST ?=

QEMU ?= OFF

define qemu_cmd
cp $(OVMF)/OVMF_VARS_4M.fd out/vars.fd
qemu-system-x86_64 -machine q35 -m $(1) -vga std $(2) \
	  -drive if=pflash,format=raw,readonly=on,file=$(OVMF)/OVMF_CODE_4M.fd \
	  -drive if=pflash,format=raw,file=out/vars.fd -drive format=raw,file=out/disk.img
endef

ifneq ($(filter ON on 1 yes true,$(QEMU)),)
run_qemu = $(call qemu_cmd,$(1),$(2))
else
define run_qemu
@echo "QEMU=OFF: built out/disk.img, not launching QEMU (use QEMU=ON)"
endef
endif

KERNEL_DST ?= vmlinuz
INITRD_DST ?= initrd.img
KFLAGS ?=
BOOT_INI = out/boot.ini

CRT_SRCS = str mem dstr iarr parr map alloc atexit format sio clock

XXC_SRCS = stream FileStream FSPath MemoryStream Map argparse app
PLAT_SRCS = crt0 alloc console fs clock
CRT_OBJS = $(addprefix out/crt_,$(CRT_SRCS:=.o)) $(addprefix out/xxc_,$(XXC_SRCS:=.o)) \
           $(addprefix out/uefi_,$(PLAT_SRCS:=.o))
DISPLAY_OBJ = out/uefi_display.o

OFFLINE_INITRD ?= out/initrd-offline.img
OFFLINE_KBASE ?= initrd=\$(INITRD_DST) quiet rdinit=/init vt.global_cursor_default=0 fbcon=nodefer

out/boot_%.o: boot/%.ppc
	@mkdir -p out
	$(CC) $(CFLAGS) -Iboot -x c -c $< -o $@
out/uefi_%.o: uefi/%.ppc
	@mkdir -p out
	$(CC) $(CFLAGS) -x c -c $< -o $@
out/crt_%.o: xxc/libminicrt/src/%.c
	@mkdir -p out
	$(CC) $(CFLAGS) -c $< -o $@
out/xxc_%.o: xxc/libxxc/src/%.ppc
	@mkdir -p out
	$(CC) $(CFLAGS) -x c -c $< -o $@
out/uefi_%.o: xxc/platform/uefi/%.c
	@mkdir -p out
	$(CC) $(CFLAGS) -c $< -o $@

out/uefi-linux.EFI: out/boot_bootloader.o $(DISPLAY_OBJ) $(CRT_OBJS)
	lld-link /subsystem:efi_application /entry:efi_main /nodefaultlib /opt:ref $^ /out:$@

$(OFFLINE_INITRD): mkinitramfs.sh FORCE
	@mkdir -p $(dir $@)
	BUSYBOX="$(BUSYBOX)" MODULES_PATH="$(MODULES)" OVERLAY="$(OVERLAY)" sh mkinitramfs.sh $@

cobalt: out/uefi-linux.EFI $(OFFLINE_INITRD) mkimage.sh
	@test -f $(KERNEL) || { echo "KERNEL=$(KERNEL) not found (needs an EFI-stub kernel)"; exit 1; }
	@mkdir -p $(dir $(BOOT_INI))
	@$(file >$(BOOT_INI),kernel=$(KERNEL_DST))
	@$(file >>$(BOOT_INI),base=$(strip $(OFFLINE_KBASE)))
	@$(file >>$(BOOT_INI),args=$(strip $(KFLAGS)))
	@echo "$(BOOT_INI)"; cat $(BOOT_INI)
	PERSIST_DIR="$(PERSIST)" EXTRA="$(KERNEL):$(KERNEL_DST) $(OFFLINE_INITRD):$(INITRD_DST) $(BOOT_INI):boot.ini" sh mkimage.sh out/uefi-linux.EFI out/disk.img
	$(call run_qemu,1024,-serial stdio $(QEMU_FLAGS))

qemu:
	@test -f out/disk.img || { echo "out/disk.img not found; run 'make cobalt' first"; exit 1; }
	$(call qemu_cmd,1024,-serial stdio $(QEMU_FLAGS))

FORCE:

.PHONY: cobalt qemu FORCE