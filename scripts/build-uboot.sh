#!/usr/bin/env bash

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# Build configuration - default using all available cores
JOBS="${JOBS:-$(nproc)}"
echo "Using $JOBS parallel job(s)"

OUTPUT_DIR="$ROOT/out/uboot"

DEFCONFIG=qemu_arm64_defconfig

ARCH="arm"
CROSS_COMPILE="aarch64-none-linux-gnu-"

mkdir -p "$OUTPUT_DIR"

cd "$ROOT/u-boot"

make -j"$JOBS" \
	O="$OUTPUT_DIR" \
	ARCH="$ARCH" \
	CROSS_COMPILE="$CROSS_COMPILE" \
	"$DEFCONFIG"

make -j"$JOBS" \
	O="$OUTPUT_DIR" \
	ARCH="$ARCH" \
	CROSS_COMPILE="$CROSS_COMPILE"
