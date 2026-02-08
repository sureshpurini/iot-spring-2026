#!/bin/bash
# run_qemu.sh - Build and run ESP32 low power example in QEMU
# IoT Course - Spring 2026
#
# Usage: ./run_qemu.sh
#
# Note: Light sleep may not work exactly as on real hardware in QEMU,
# but the code demonstrates the concepts. The example includes a
# fallback mode for QEMU compatibility.

set -e

APP_NAME="espidf_low_power"
APP_BIN="build/${APP_NAME}.bin"

echo "========================================"
echo "ESP32 Low Power Example - QEMU Runner"
echo "========================================"
echo ""

# Check if build exists
if [ ! -f "$APP_BIN" ]; then
    echo "Binary not found. Building project..."
    idf.py build
fi

echo "Creating 4MB flash image..."
esptool.py --chip esp32 merge_bin \
    --fill-flash-size 4MB \
    -o build/flash.bin \
    --flash_mode dio \
    --flash_size 4MB \
    0x1000 build/bootloader/bootloader.bin \
    0x8000 build/partition_table/partition-table.bin \
    0x10000 "$APP_BIN"

echo ""
echo "Starting QEMU..."
echo "Exit with: Ctrl+A, X"
echo "(In screen: Ctrl+A, A, X)"
echo ""

qemu-system-xtensa -nographic \
    -machine esp32 \
    -drive file=build/flash.bin,if=mtd,format=raw
