#!/usr/bin/env bash

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

QEMU=qemu-system-aarch64

KERNEL_UPSTREAM_IMAGE="/home/zephyr/SecureSWCodebase/optee_qemu_a64/out/bin/Image"
KERNEL_IMAGE="$ROOT/out/linux/arch/arm64/boot/Image"

ROOTFS_UPSTREAM_IMAGE="/home/zephyr/SecureSWCodebase/optee_qemu_a64/out/bin/rootfs.cpio.gz"
ROOTFS_IMAGE="$ROOT/out/buildroot/images/rootfs.cpio.gz"
#TFA_OUT_DIR=out/tf-a/qemu/release

TFA_OUT_DIR=out/tf-a/qemu/debug

"$QEMU" \
	-machine virt,secure=on,gic-version=2 \
	-cpu max \
	-smp 4 \
	-m 1024 \
	-display gtk \
	-chardev pty,id=secure_uart \
	-serial stdio \
	-serial chardev:secure_uart \
	-device virtio-gpu \
	-kernel "$KERNEL_IMAGE" \
	-initrd "$ROOTFS_IMAGE" \
	-append "console=ttyAMA0,38400 keep_bootcon" \
	-bios "$TFA_OUT_DIR/qemu_fw.bios" \
	-d guest_errors -d unimp \
	-semihosting-config enable=on,target=native \
