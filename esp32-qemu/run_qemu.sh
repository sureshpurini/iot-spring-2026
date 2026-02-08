#!/bin/bash
# Helper script to build and run ESP32 project in QEMU

set -e

# Find the project binary name
if [ -f "build/blink.bin" ]; then
    APP_BIN="build/blink.bin"
elif [ -f "build/espidf_blink.bin" ]; then
    APP_BIN="build/espidf_blink.bin"
else
    APP_BIN=$(ls build/*.bin 2>/dev/null | grep -v bootloader | grep -v partition | head -1)
fi

echo "Using app binary: $APP_BIN"

echo "Creating 4MB flash image..."
esptool.py --chip esp32 merge_bin \
    --fill-flash-size 4MB \
    -o build/flash_image.bin \
    --flash_mode dio \
    --flash_size 4MB \
    0x1000 build/bootloader/bootloader.bin \
    0x8000 build/partition_table/partition-table.bin \
    0x10000 $APP_BIN

echo "Starting QEMU (Ctrl+A, X to exit)..."
qemu-system-xtensa -nographic \
    -machine esp32 \
    -drive file=build/flash_image.bin,if=mtd,format=raw
