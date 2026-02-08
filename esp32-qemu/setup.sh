#!/bin/bash
# ESP32 QEMU Environment Setup Script
# Run this once to build the Docker image

set -e

echo "=========================================="
echo "  ESP32 QEMU Development Environment"
echo "  IoT Course - Spring 2026"
echo "=========================================="
echo ""

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Error: Docker is not installed."
    echo "Please install Docker Desktop from https://www.docker.com/products/docker-desktop"
    exit 1
fi

# Check if Docker is running
if ! docker info &> /dev/null; then
    echo "Error: Docker is not running."
    echo "Please start Docker Desktop and try again."
    exit 1
fi

echo "Building Docker image (this may take 15-20 minutes on first run)..."
echo ""

docker compose build

echo ""
echo "=========================================="
echo "  Setup Complete!"
echo "=========================================="
echo ""
echo "To start the development environment, run:"
echo "  ./start.sh"
echo ""
echo "To build and run the hello-world example:"
echo "  ./start.sh"
echo "  # Inside container:"
echo "  cd projects/01-hello-world"
echo "  /workspace/scripts/build-and-run.sh ."
echo ""
