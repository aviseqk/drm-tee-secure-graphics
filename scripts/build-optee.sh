#!/usr/bin/env bash

set -euo pipefail

ROOT="$(cd "$(dirname "$0")"/.. && pwd)"

# Build configuration - default using all available cores
JOBS="${JOBS:-$(nproc)}"
echo "Using $JOBS parallel job(s)"

OUTPUT_DIR="$ROOT/out/optee"

PLATFORM="vexpress-qemu_armv8a"
ARCH="arm"
CROSS_COMPILE="aarch64-none-linux-gnu-"
BUILD_TYPE="release"

mkdir -p "$OUTPUT_DIR"

cd "$ROOT/optee_os"


# Restrict OP-TEE to build only the ARM64 TA variant.
# By default, CFG_ARM64_CORE=y builds both ta_arm64 and ta_arm32.
make -j"$JOBS" \
	O="$OUTPUT_DIR" \
	PLATFORM="$PLATFORM" \
	ARCH="$ARCH" \
	CROSS_COMPILE64="$CROSS_COMPILE" \
	CFG_ARM64_core=y \
	CFG_ARM32_core=n \
	CFG_USER_TA_TARGETS=ta_arm64 \
#	DEBUG=0

