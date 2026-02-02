#!/bin/bash

echo "=========================================="
echo "  Running Tests in Docker"
echo "=========================================="
echo ""

# Stop any running containers
docker-compose down 2>/dev/null

# Remove old images to force rebuild
echo "→ Cleaning old Docker artifacts..."
docker-compose rm -f 2>/dev/null

# Build fresh image
echo "→ Building Docker image..."
docker-compose build --no-cache

# Run tests
echo ""
echo "→ Running tests..."
echo ""
docker-compose up --abort-on-container-exit

# Capture exit code
EXIT_CODE=$?

# Cleanup
echo ""
echo "→ Cleaning up..."
docker-compose down

if [ $EXIT_CODE -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "  ✓✓✓ SUCCESS! All tests passed!"
    echo "=========================================="
    exit 0
else
    echo ""
    echo "=========================================="
    echo "  ✗✗✗ FAILURE! Some tests failed."
    echo "=========================================="
    exit 1
fi
