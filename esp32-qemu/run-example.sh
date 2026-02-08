#!/bin/bash
# Run an example project in the ESP32 QEMU environment
# Usage: ./run-example.sh <project-name>
# Example: ./run-example.sh 01-hello-world

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

PROJECT=${1:-01-hello-world}

if [ ! -d "projects/${PROJECT}" ]; then
    echo "Error: Project '${PROJECT}' not found in projects/"
    echo ""
    echo "Available projects:"
    ls -1 projects/
    exit 1
fi

echo "=========================================="
echo "  Building and running: ${PROJECT}"
echo "=========================================="

docker compose run --rm esp32-dev /workspace/scripts/build-and-run.sh /workspace/projects/${PROJECT}
