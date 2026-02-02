#!/bin/bash
set -e

echo "================================================"
echo "  Building and Testing in Docker"
echo "================================================"

# Build Docker image
echo "→ Building Docker image..."
docker-compose build

echo ""
echo "→ Running tests in Docker..."
echo ""

# Run tests (this will use the command from docker-compose.yml)
docker-compose up --abort-on-container-exit

# Capture exit code
EXIT_CODE=$?

# Cleanup
docker-compose down

echo ""
if [ $EXIT_CODE -eq 0 ]; then
    echo "================================================"
    echo "  ✓ Docker Build & Test Successful!"
    echo "================================================"
    exit 0
else
    echo "================================================"
    echo "  ✗ Docker Build or Tests Failed!"
    echo "================================================"
    exit 1
fi
