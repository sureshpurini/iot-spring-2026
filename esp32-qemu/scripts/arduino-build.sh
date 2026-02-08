#!/bin/bash
# Build an Arduino sketch for ESP32 QEMU
# Usage: ./arduino-build.sh <sketch_path>
# Example: ./arduino-build.sh /workspace/arduino/01-blink

set -e

SKETCH_PATH=${1:-.}
SKETCH_NAME=$(basename "${SKETCH_PATH}")

if [ ! -d "${SKETCH_PATH}" ]; then
    echo "Error: Sketch directory '${SKETCH_PATH}' not found"
    exit 1
fi

# Find the .ino file
INO_FILE=$(find "${SKETCH_PATH}" -name "*.ino" -type f | head -1)
if [ -z "${INO_FILE}" ]; then
    echo "Error: No .ino file found in ${SKETCH_PATH}"
    exit 1
fi

echo "=========================================="
echo "Building Arduino sketch: ${SKETCH_NAME}"
echo "=========================================="

# Build output goes into a build/ directory inside the sketch folder
BUILD_DIR="${SKETCH_PATH}/build"
mkdir -p "${BUILD_DIR}"

# Compile with arduino-cli for ESP32
arduino-cli compile \
    --fqbn esp32:esp32:esp32 \
    --build-path "${BUILD_DIR}" \
    "${SKETCH_PATH}"

echo "=========================================="
echo "Build complete!"
echo "Binary: ${BUILD_DIR}/${SKETCH_NAME}.ino.bin"
echo "Bootloader: ${BUILD_DIR}/${SKETCH_NAME}.ino.bootloader.bin"
echo "Partitions: ${BUILD_DIR}/${SKETCH_NAME}.ino.partitions.bin"
echo "=========================================="
