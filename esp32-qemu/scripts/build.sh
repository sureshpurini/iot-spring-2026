#!/bin/bash
# Build an ESP32 project for QEMU
# Usage: ./build.sh <project_path>

set -e

PROJECT_PATH=${1:-.}

if [ ! -f "${PROJECT_PATH}/CMakeLists.txt" ]; then
    echo "Error: No CMakeLists.txt found in ${PROJECT_PATH}"
    echo "Usage: ./build.sh <project_path>"
    exit 1
fi

cd "${PROJECT_PATH}"

echo "=========================================="
echo "Building ESP32 project: $(basename $(pwd))"
echo "=========================================="

# Set target to ESP32
idf.py set-target esp32

# Build the project
idf.py build

echo "=========================================="
echo "Build complete!"
echo "Binary: build/$(basename $(pwd)).bin"
echo "=========================================="
