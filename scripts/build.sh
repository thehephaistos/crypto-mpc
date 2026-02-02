#!/bin/bash
set -e  # Exit on any error

echo "================================================"
echo "  Building Shamir's Secret Sharing Library"
echo "================================================"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check dependencies
echo -e "${BLUE}→ Checking dependencies...${NC}"

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}✗ CMake not found. Please install CMake.${NC}"
    exit 1
fi
echo -e "${GREEN}✓ CMake found${NC}"

if ! command -v make &> /dev/null; then
    echo -e "${RED}✗ Make not found. Please install Make.${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Make found${NC}"

# Check for libsodium (try to compile a test)
echo -e "${BLUE}→ Checking for libsodium...${NC}"
cat > /tmp/test_sodium.c << 'EOF'
#include <sodium.h>
int main() { return sodium_init(); }
EOF

if gcc -o /tmp/test_sodium /tmp/test_sodium.c -lsodium 2>/dev/null; then
    echo -e "${GREEN}✓ libsodium found${NC}"
    rm -f /tmp/test_sodium /tmp/test_sodium.c
else
    echo -e "${RED}✗ libsodium not found. Please install libsodium.${NC}"
    echo -e "${BLUE}Install with:${NC}"
    echo -e "  macOS:   brew install libsodium"
    echo -e "  Ubuntu:  sudo apt install libsodium-dev"
    echo -e "  Fedora:  sudo dnf install libsodium-devel"
    rm -f /tmp/test_sodium.c
    exit 1
fi

# Create build directory
echo -e "${BLUE}→ Creating build directory...${NC}"
mkdir -p build
cd build

# Run CMake
echo -e "${BLUE}→ Running CMake...${NC}"
cmake .. || { echo -e "${RED}✗ CMake failed${NC}"; exit 1; }
echo -e "${GREEN}✓ CMake completed${NC}"

# Build
echo -e "${BLUE}→ Building project...${NC}"
make || { echo -e "${RED}✗ Build failed${NC}"; exit 1; }
echo -e "${GREEN}✓ Build completed${NC}"

echo ""
echo -e "${GREEN}================================================${NC}"
echo -e "${GREEN}  Build Successful!${NC}"
echo -e "${GREEN}================================================${NC}"
echo ""
echo "Run tests:"
echo "  cd build"
echo "  ./simple_test"
echo "  ./comprehensive_test"
echo "  ./secure_memory_test"
echo "  ./mpc_foundation_test"
