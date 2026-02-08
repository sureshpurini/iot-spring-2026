#!/bin/bash
# Build and run an Arduino sketch in QEMU
# Usage: ./arduino-build-and-run.sh <sketch_path>

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKETCH_PATH=${1:-.}

# Build the sketch
"${SCRIPT_DIR}/arduino-build.sh" "${SKETCH_PATH}"

# Run in QEMU
"${SCRIPT_DIR}/arduino-run-qemu.sh" "${SKETCH_PATH}"
