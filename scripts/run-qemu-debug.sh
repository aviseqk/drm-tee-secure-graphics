#!/usr/bin/env bash

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

QEMU=qemu-system-aarch64

TFA_OUT_DIR=out/tf-a/qemu/release

"$QEMU" \
	-machine virt,secure=on,gic-version=3 \
	-cpu max \
	-smp 4 \
	-m 1024 \
	-nographic \
	-bios "$TFA_OUT_DIR/qemu_fw.bios" \
	-d guest_errors \
	-semihosting-config enable=on,target=native \
	-S \
	-s
