#!/bin/bash
# Build and run an ESP32 project in QEMU
# Usage: ./build-and-run.sh <project_path>

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_PATH=${1:-.}

# Build the project
"${SCRIPT_DIR}/build.sh" "${PROJECT_PATH}"

# Run in QEMU
"${SCRIPT_DIR}/run-qemu.sh" "${PROJECT_PATH}"
