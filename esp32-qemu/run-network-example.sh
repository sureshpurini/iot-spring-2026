#!/bin/bash
# Run a network-enabled ESP32 project in QEMU with supporting services
# Usage: ./run-network-example.sh <project-name>
# Example: ./run-network-example.sh 03-rest-api
#          ./run-network-example.sh 04-mqtt
#
# This script:
# 1. Starts supporting services (api-server, mqtt-broker) based on the project
# 2. Builds the ESP32 project inside Docker
# 3. Runs it in QEMU with networking enabled (open_eth via slirp)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

PROJECT=${1:-03-rest-api}

if [ ! -d "projects/${PROJECT}" ]; then
    echo "Error: Project '${PROJECT}' not found in projects/"
    echo ""
    echo "Available projects:"
    ls -1 projects/
    exit 1
fi

echo "=========================================="
echo "  Network-enabled ESP32 Project: ${PROJECT}"
echo "=========================================="

# Step 1: Start the appropriate backend services
SERVICES=""
echo ""
echo "[1/3] Starting backend services..."

case "${PROJECT}" in
    *rest-api*)
        SERVICES="api-server"
        docker compose up -d api-server
        sleep 2
        if docker compose ps api-server 2>/dev/null | grep -q "running"; then
            echo "  API server is running at http://localhost:5000"
            curl -s http://localhost:5000/health 2>/dev/null && echo "" || true
        fi
        ;;
    *mqtt*)
        SERVICES="mqtt-broker"
        docker compose up -d mqtt-broker
        sleep 2
        if docker compose ps mqtt-broker 2>/dev/null | grep -q "running"; then
            echo "  MQTT broker is running at localhost:1883"
        fi
        ;;
    *)
        # Generic: try starting all backend services
        SERVICES="api-server mqtt-broker"
        docker compose up -d api-server mqtt-broker 2>/dev/null || true
        sleep 2
        ;;
esac

# Step 2: Build and run the ESP32 project with networking
echo ""
echo "[2/3] Building ESP32 project..."
echo "[3/3] Starting QEMU with networking..."
echo ""

docker compose run --rm \
    -e QEMU_NET=1 \
    esp32-dev \
    /workspace/scripts/build-and-run.sh /workspace/projects/${PROJECT}

# Step 3: Cleanup — stop backend services
echo ""
echo "Stopping backend services..."
for svc in ${SERVICES}; do
    docker compose stop ${svc} 2>/dev/null || true
done
