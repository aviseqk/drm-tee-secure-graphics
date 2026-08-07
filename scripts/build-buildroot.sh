#!/usr/bin/env bash

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

JOBS="${JOBS:-$(nproc)}"
echo "Using $JOBS parallel job(s)"

BR_SOURCE="$ROOT/buildroot"
OUTPUT_DIR="$ROOT/out/buildroot"

DEFCONFIG="$ROOT/configs/buildroot/secure_graphics_qemu_defconfig"

echo "Using Buildroot defconfig:"
echo "  $DEFCONFIG"

mkdir -p "$OUTPUT_DIR"

cd "$BR_SOURCE"

make \
    O="$OUTPUT_DIR" \
    BR2_EXTERNAL="$ROOT/sg-ext" \
    BR2_DEFCONFIG="$DEFCONFIG" \
    defconfig

make -j"$JOBS" \
    O="$OUTPUT_DIR" \
    BR2_EXTERNAL="$ROOT/sg-ext"
