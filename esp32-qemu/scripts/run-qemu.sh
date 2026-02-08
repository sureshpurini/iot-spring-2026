#!/bin/bash
# Run an ESP32 project in QEMU
# Usage: ./run-qemu.sh <project_path>

set -e

PROJECT_PATH=${1:-.}
PROJECT_NAME=$(basename "${PROJECT_PATH}")

# Find the binary
if [ -f "${PROJECT_PATH}/build/${PROJECT_NAME}.bin" ]; then
    BINARY="${PROJECT_PATH}/build/${PROJECT_NAME}.bin"
elif [ -f "${PROJECT_PATH}/build/app.bin" ]; then
    BINARY="${PROJECT_PATH}/build/app.bin"
else
    # Try to find any .bin file
    BINARY=$(find "${PROJECT_PATH}/build" -name "*.bin" -type f | head -1)
fi

if [ -z "${BINARY}" ] || [ ! -f "${BINARY}" ]; then
    echo "Error: No binary found. Did you build the project first?"
    echo "Run: ./build.sh ${PROJECT_PATH}"
    exit 1
fi

# Merge binaries for QEMU (bootloader + partition table + app)
MERGED_BIN="${PROJECT_PATH}/build/merged-qemu.bin"

echo "=========================================="
echo "Preparing binary for QEMU..."
echo "=========================================="

cd "${PROJECT_PATH}"

# Create merged binary using esptool
python3 -m esptool --chip esp32 merge_bin \
    --fill-flash-size 4MB \
    -o build/merged-qemu.bin \
    --flash_mode dio \
    --flash_size 4MB \
    0x1000 build/bootloader/bootloader.bin \
    0x8000 build/partition_table/partition-table.bin \
    0x10000 build/*.bin 2>/dev/null || \
python3 -m esptool --chip esp32 merge_bin \
    --fill-flash-size 4MB \
    -o build/merged-qemu.bin \
    --flash_mode dio \
    --flash_size 4MB \
    0x1000 build/bootloader/bootloader.bin \
    0x8000 build/partition_table/partition-table.bin \
    0x10000 build/app-template.bin 2>/dev/null || \
echo "Using existing binary..."

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
    -drive file=build/merged-qemu.bin,if=mtd,format=raw \
    -serial mon:stdio \
    ${NETWORK_ARGS}
