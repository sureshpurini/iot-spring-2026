#!/bin/bash
# Run an Arduino sketch in ESP32 QEMU
# Usage: ./run-arduino-example.sh <sketch-name>
# Example: ./run-arduino-example.sh 01-blink
#          ./run-arduino-example.sh 02-serial-output

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

SKETCH=${1:-01-blink}

if [ ! -d "../arduino/${SKETCH}" ]; then
    echo "Error: Sketch '${SKETCH}' not found in arduino/"
    echo ""
    echo "Available sketches:"
    ls -1 ../arduino/ | grep -v README
    exit 1
fi

echo "=========================================="
echo "  Arduino in QEMU: ${SKETCH}"
echo "=========================================="

docker compose run --rm \
    esp32-dev \
    /workspace/scripts/arduino-build-and-run.sh /workspace/arduino/${SKETCH}
