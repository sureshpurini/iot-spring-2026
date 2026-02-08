#!/bin/bash
# Run a compiled Arduino sketch in QEMU
# Usage: ./arduino-run-qemu.sh <sketch_path>

set -e

SKETCH_PATH=${1:-.}
SKETCH_NAME=$(basename "${SKETCH_PATH}")
BUILD_DIR="${SKETCH_PATH}/build"

# Find the compiled binary
APP_BIN="${BUILD_DIR}/${SKETCH_NAME}.ino.bin"
BOOT_BIN="${BUILD_DIR}/${SKETCH_NAME}.ino.bootloader.bin"
PART_BIN="${BUILD_DIR}/${SKETCH_NAME}.ino.partitions.bin"

if [ ! -f "${APP_BIN}" ]; then
    echo "Error: No compiled binary found at ${APP_BIN}"
    echo "Run arduino-build.sh first"
    exit 1
fi

echo "=========================================="
echo "Preparing Arduino binary for QEMU..."
echo "=========================================="

# Merge binaries into a single flash image for QEMU
python3 -m esptool --chip esp32 merge_bin \
    --fill-flash-size 4MB \
    -o "${BUILD_DIR}/merged-qemu.bin" \
    --flash_mode dio \
    --flash_size 4MB \
    0x1000 "${BOOT_BIN}" \
    0x8000 "${PART_BIN}" \
    0x10000 "${APP_BIN}"

echo "=========================================="
echo "Starting QEMU ESP32 emulator..."
echo "Press Ctrl+A then X to exit QEMU"
echo "=========================================="

# Check if networking is requested
NETWORK_ARGS=""
if [ "${QEMU_NET:-0}" = "1" ]; then
    echo "Networking enabled (open_eth via slirp)"
    NETWORK_ARGS="-nic user,model=open_eth"
fi

# Run QEMU
qemu-system-xtensa \
    -nographic \
    -machine esp32 \
    -drive file="${BUILD_DIR}/merged-qemu.bin",if=mtd,format=raw \
    -serial mon:stdio \
    ${NETWORK_ARGS}
