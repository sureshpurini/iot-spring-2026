#!/bin/bash
# Start the ESP32 QEMU development environment

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

# Check if Docker is running
if ! docker info &> /dev/null; then
    echo "Error: Docker is not running."
    echo "Please start Docker Desktop and try again."
    exit 1
fi

# Check if image exists
if ! docker images | grep -q "esp32-qemu-dev"; then
    echo "Docker image not found. Running setup first..."
    ./setup.sh
fi

echo "=========================================="
echo "  Starting ESP32 QEMU Environment"
echo "=========================================="
echo ""
echo "Useful commands inside the container:"
echo "  cd projects/01-hello-world     # Go to example project"
echo "  /workspace/scripts/build.sh .  # Build current project"
echo "  /workspace/scripts/run-qemu.sh . # Run in QEMU"
echo ""
echo "To exit: type 'exit' or press Ctrl+D"
echo "To exit QEMU: press Ctrl+A then X"
echo "=========================================="
echo ""

docker compose run --rm esp32-dev
