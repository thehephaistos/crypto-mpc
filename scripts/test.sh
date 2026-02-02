#!/bin/bash
set -e

echo "================================================"
echo "  Running All Tests"
echo "================================================"

GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

cd build

TESTS=(
    "simple_test"
    "comprehensive_test"
    "secure_memory_test"
    "mpc_foundation_test"
)

PASSED=0
FAILED=0

for test in "${TESTS[@]}"; do
    if [ -f "./$test" ]; then
        echo -e "${BLUE}→ Running $test...${NC}"
        if "./$test"; then
            echo -e "${GREEN}✓ $test PASSED${NC}"
            ((PASSED++))
        else
            echo -e "${RED}✗ $test FAILED${NC}"
            ((FAILED++))
        fi
        echo ""
    else
        echo -e "${RED}✗ $test not found${NC}"
        ((FAILED++))
    fi
done

echo "================================================"
echo "  Test Results"
echo "================================================"
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"
echo "================================================"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ ALL TESTS PASSED!${NC}"
    exit 0
else
    echo -e "${RED}✗ SOME TESTS FAILED${NC}"
    exit 1
fi
