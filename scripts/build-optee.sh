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
COMMON_ARGS=(
	-j"$JOBS"
	O="$OUTPUT_DIR"
	PLATFORM="$PLATFORM"
	ARCH="$ARCH"
	CROSS_COMPILE64="$CROSS_COMPILE"
	CFG_ARM64_core=y
	CFG_ARM32_core=n
	CFG_USER_TA_TARGETS=ta_arm64
)

################################################################
# Stage 1 Build: Build OPTEE to get the TA Development Kit
################################################################

if [[ "${1:-}" == "devkit" ]]; then
    echo "Building OP-TEE TA dev kit..."

    make "${COMMON_ARGS[@]}" \
        CFG_EARLY_TA=y \
        ta_dev_kit

    exit 0
fi


###################################################################
# Stage 2 Build: Build OP-TEE with the secure-graphics TA embedded
###################################################################

# using EARLY_TA_PATHS as the way to embed an out-of-tree built TA into the OPTEE OS Image
EARLY_TA="$ROOT/out/buildroot/build/sg-tee-test-1.0/ta/out/cc6c3285-e725-4249-bfc4-3ad5a558e051.stripped.elf"

if [[ ! -f "$EARLY_TA" ]]; then
	echo "ERROR: early TA not found:"
	echo " $EARLY_TA "
	echo "Build sg-tee-test first using buildroot sg-ext"
	exit 1
fi

echo "Using EARLY TA:"
echo "  $EARLY_TA"

make "${COMMON_ARGS[@]}" \
    EARLY_TA_PATHS="$EARLY_TA"
