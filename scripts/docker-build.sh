#!/bin/bash
set -e

echo "================================================"
echo "  Building in Docker"
echo "================================================"

# Build Docker image
echo "→ Building Docker image..."
docker-compose build

echo ""
echo "================================================"
echo "  Running Tests in Docker"
echo "================================================"

# Run tests in container
docker-compose run --rm crypto-lib bash -c "cd build && ./simple_test && ./comprehensive_test && ./secure_memory_test && ./mpc_foundation_test"

echo ""
echo "================================================"
echo "  Docker Build & Test Successful!"
echo "================================================"
