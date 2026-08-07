#!/usr/bin/env bash

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# Build configuration - default using all available cores
JOBS="${JOBS:-$(nproc)}"
echo "Using $JOBS parallel job(s)"

BUILD_BASE="$ROOT/out/tf-a"
PLAT=qemu
ARCH=aarch64
CROSS_COMPILE=aarch64-none-linux-gnu-
BUILD_TYPE=debug

# TF-A platform variable - for the GICv3 version mismatch issue
QEMU_GIC_VERSION="QEMU_GICV2"

# OP-TEE artifacts
SPD=opteed
BL32="$ROOT/out/optee/core/tee-header_v2.bin"
BL32_EXTRA1="$ROOT/out/optee/core/tee-pager_v2.bin"
BL32_EXTRA2="$ROOT/out/optee/core/tee-pageable_v2.bin"

# UBoot artifacts
BL33="$ROOT/out/uboot/u-boot.bin"

if [[ ! -f "$BL32" ]]; then
	echo "ERROR: BL32 image not found!:"
	echo " $BL32"
	exit 1
fi

echo "SUCCESS: Found BL32 image:"
echo " $BL32"

mkdir -p $BUILD_BASE

cd "$ROOT/trusted-firmware-a"

#BUILD_TYPE=debug
make -j"$JOBS" \
	PLAT=$PLAT \
	ARCH=$ARCH \
	LOG_LEVEL=40 \
	QEMU_USE_GIC_DRIVER=$QEMU_GIC_VERSION \
	CROSS_COMPILE=$CROSS_COMPILE \
	BUILD_TYPE=$BUILD_TYPE \
	BUILD_BASE=$BUILD_BASE \
	SPD="$SPD" \
	BL32="$BL32" BL32_EXTRA1="$BL32_EXTRA1" BL32_EXTRA2="$BL32_EXTRA2" \
	BL33="$BL33"
