#!/usr/bin/env bash

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

JOBS="${JOBS:-$(nproc)}"

LINUX_SRC="$ROOT/linux"
OUT_DIR="$ROOT/out/linux"

FRAGMENT="$ROOT/configs/linux/qemu-optee.fragment"


LINUX_TARGETS=(Image)
LINUX_TARGETS+=(scripts_gdb)

ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

mkdir -p "$OUT_DIR"

echo "Using $JOBS parallel job(s)"


# navigate to linux source directory and merge our fragment with the arch base defconfig to create final .config
cd "$LINUX_SRC"

ARCH="$ARCH" \
CROSS_COMPILE="$CROSS_COMPILE" \
scripts/kconfig/merge_config.sh \
    -O "$OUT_DIR" \
    arch/arm64/configs/defconfig \
    "$FRAGMENT"

### NOTE: the following commented steps are what we were doing earlier but in here we were not actually
### merging the arch/arm64 base defconfig, we were just doing "make defconfig" step and then merging it with
### our fragment conf, hence missing out on the arch/arm64 defconfigs and thus missing a lot of necessary drivers and modules
#make \
#	-C "$LINUX_SRC" \
#	O="$OUT_DIR" \
#	ARCH="$ARCH" \
#	CROSS_COMPILE="$CROSS_COMPILE" \
#	defconfig

# merge our defconfig fragment

#make \
#	-C "$LINUX_SRC" \
#	O="$OUT_DIR" \
#	ARCH="$ARCH" \
#	CROSS_COMPILE="$CROSS_COMPILE" \
#	KCONFIG_ALLCONFIG="$FRAGMENT" \
#	alldefconfig

# build the linux Image artifact
make -j"$JOBS" \
	-C "$LINUX_SRC" \
	O="$OUT_DIR" \
	ARCH="$ARCH" \
	CROSS_COMPILE="$CROSS_COMPILE" \
	"${LINUX_TARGETS[@]}"
