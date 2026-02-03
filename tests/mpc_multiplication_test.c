#include "sss/mpc.h"
#include "sss/secret_sharing.h"
#include "sss/field.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ANSI color codes
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, test_name) do { \
    if (condition) { \
        printf("  " COLOR_GREEN "✓ PASS: " COLOR_RESET "%s\n", test_name); \
        tests_passed++; \
    } else { \
        printf("  " COLOR_RED "✗ FAIL: " COLOR_RESET "%s\n", test_name); \
        tests_failed++; \
    } \
} while(0)

void print_separator() {
    printf(COLOR_CYAN "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" COLOR_RESET);
}

void print_header(const char *title) {
    printf("\n");
    print_separator();
    printf(COLOR_BLUE "  %s\n" COLOR_RESET, title);
    print_separator();
}

// Test 1: Basic Multiplication
int test_basic_multiplication() {
    printf("\n" COLOR_YELLOW "→ Test 1: Basic Multiplication (5 × 6 = 30)" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    if (mpc_init_context(&ctx, 5, 3, 1) != 0) {
        return 0;
    }
    
    // Secrets: 5 × 6 = 30
    uint8_t secret_a = 5;
    uint8_t secret_b = 6;
    
    mpc_share_t shares_a[5], shares_b[5];
    mpc_create_shares(&ctx, &secret_a, shares_a);
    mpc_create_shares(&ctx, &secret_b, shares_b);
    
    // Multiply
    mpc_share_t shares_prod[5];
    if (mpc_secure_mul(&ctx, shares_a, shares_b, shares_prod, 5) != 0) {
        printf("  " COLOR_RED "Multiplication failed!\n" COLOR_RESET);
        mpc_cleanup_context(&ctx);
        return 0;
    }
    
    // Reconstruct
    uint8_t result;
    mpc_reconstruct(&ctx, shares_prod, 3, &result);
    
    printf("  Secret A: %d\n", secret_a);
    printf("  Secret B: %d\n", secret_b);
    printf("  Expected: 30\n");
    printf("  Actual: %d\n", result);
    
    int success = (result == 30);
    
    // Cleanup
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares_a[i]);
        mpc_wipe_share(&shares_b[i]);
        mpc_wipe_share(&shares_prod[i]);
    }
    mpc_cleanup_context(&ctx);
    
    return success;
}

// Test 2: Multiplication by Zero
int test_multiply_by_zero() {
    printf("\n" COLOR_YELLOW "→ Test 2: Multiplication by Zero (50 × 0 = 0)" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t secret_a = 50;
    uint8_t secret_b = 0;
    
    mpc_share_t shares_a[5], shares_b[5];
    mpc_create_shares(&ctx, &secret_a, shares_a);
    mpc_create_shares(&ctx, &secret_b, shares_b);
    
    mpc_share_t shares_prod[5];
    mpc_secure_mul(&ctx, shares_a, shares_b, shares_prod, 5);
    
    uint8_t result;
    mpc_reconstruct(&ctx, shares_prod, 3, &result);
    
    printf("  %d × 0 = %d (expected 0)\n", secret_a, result);
    
    int success = (result == 0);
    
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares_a[i]);
        mpc_wipe_share(&shares_b[i]);
        mpc_wipe_share(&shares_prod[i]);
    }
    mpc_cleanup_context(&ctx);
    
    return success;
}

// Test 3: Multiplication by One
int test_multiply_by_one() {
    printf("\n" COLOR_YELLOW "→ Test 3: Multiplication by One (42 × 1 = 42)" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t secret_a = 42;
    uint8_t secret_b = 1;
    
    mpc_share_t shares_a[5], shares_b[5];
    mpc_create_shares(&ctx, &secret_a, shares_a);
    mpc_create_shares(&ctx, &secret_b, shares_b);
    
    mpc_share_t shares_prod[5];
    mpc_secure_mul(&ctx, shares_a, shares_b, shares_prod, 5);
    
    uint8_t result;
    mpc_reconstruct(&ctx, shares_prod, 3, &result);
    
    printf("  42 × 1 = %d (expected 42)\n", result);
    
    int success = (result == 42);
    
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares_a[i]);
        mpc_wipe_share(&shares_b[i]);
        mpc_wipe_share(&shares_prod[i]);
    }
    mpc_cleanup_context(&ctx);
    
    return success;
}

// Test 4: Large Numbers in GF(256)
int test_large_numbers() {
    printf("\n" COLOR_YELLOW "→ Test 4: Large Numbers (100 × 2)" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t secret_a = 100;
    uint8_t secret_b = 2;
    
    mpc_share_t shares_a[5], shares_b[5];
    mpc_create_shares(&ctx, &secret_a, shares_a);
    mpc_create_shares(&ctx, &secret_b, shares_b);
    
    mpc_share_t shares_prod[5];
    mpc_secure_mul(&ctx, shares_a, shares_b, shares_prod, 5);
    
    uint8_t result;
    mpc_reconstruct(&ctx, shares_prod, 3, &result);
    
    // In GF(256), multiplication wraps around
    uint8_t expected = gf256_mul(100, 2);
    
    printf("  100 × 2 = %d (in GF(256): %d)\n", result, expected);
    
    int success = (result == expected);
    
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares_a[i]);
        mpc_wipe_share(&shares_b[i]);
        mpc_wipe_share(&shares_prod[i]);
    }
    mpc_cleanup_context(&ctx);
    
    return success;
}

// Test 5: Chained Multiplication ((3 × 4) × 5 = 60)
int test_chained_multiplication() {
    printf("\n" COLOR_YELLOW "→ Test 5: Chained Multiplication ((3 × 4) × 5)" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t a = 3, b = 4, c = 5;
    
    mpc_share_t shares_a[5], shares_b[5], shares_c[5];
    mpc_create_shares(&ctx, &a, shares_a);
    mpc_create_shares(&ctx, &b, shares_b);
    mpc_create_shares(&ctx, &c, shares_c);
    
    // First: a × b
    mpc_share_t shares_temp[5];
    mpc_secure_mul(&ctx, shares_a, shares_b, shares_temp, 5);
    
    // Then: (a×b) × c
    mpc_share_t shares_result[5];
    mpc_secure_mul(&ctx, shares_temp, shares_c, shares_result, 5);
    
    uint8_t result;
    mpc_reconstruct(&ctx, shares_result, 3, &result);
    
    printf("  (3 × 4) × 5 = %d (expected 60)\n", result);
    
    int success = (result == 60);
    
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares_a[i]);
        mpc_wipe_share(&shares_b[i]);
        mpc_wipe_share(&shares_c[i]);
        mpc_wipe_share(&shares_temp[i]);
        mpc_wipe_share(&shares_result[i]);
    }
    mpc_cleanup_context(&ctx);
    
    return success;
}

// Test 6: Combined Operations ((10 × 5) + 20 = 70)
int test_combined_operations() {
    printf("\n" COLOR_YELLOW "→ Test 6: Combined Operations ((10 × 5) + 20)" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t a = 10, b = 5, c = 20;
    
    mpc_share_t shares_a[5], shares_b[5], shares_c[5];
    mpc_create_shares(&ctx, &a, shares_a);
    mpc_create_shares(&ctx, &b, shares_b);
    mpc_create_shares(&ctx, &c, shares_c);
    
    // First: a × b
    mpc_share_t shares_prod[5];
    mpc_secure_mul(&ctx, shares_a, shares_b, shares_prod, 5);
    
    // Then: (a×b) + c
    mpc_share_t shares_result[5];
    mpc_secure_add(&ctx, shares_prod, shares_c, shares_result, 5);
    
    uint8_t result;
    mpc_reconstruct(&ctx, shares_result, 3, &result);
    
    // In GF(256): 10 × 5 = 50, then 50 XOR 20 = 54 (not 70!)
    uint8_t expected = gf256_add(gf256_mul(a, b), c);
    
    printf("  (10 × 5) + 20 = %d (in GF(256): %d)\n", result, expected);
    
    int success = (result == expected);
    
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares_a[i]);
        mpc_wipe_share(&shares_b[i]);
        mpc_wipe_share(&shares_c[i]);
        mpc_wipe_share(&shares_prod[i]);
        mpc_wipe_share(&shares_result[i]);
    }
    mpc_cleanup_context(&ctx);
    
    return success;
}

// Test 7: Real-World - Calculate Area (Length × Width)
int test_calculate_area() {
    printf("\n" COLOR_YELLOW "→ Test 7: Real-World - Calculate Rectangle Area" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    // Secret dimensions: 12 × 8 = 96
    uint8_t length = 12;
    uint8_t width = 8;
    
    printf("  Secret length: %d meters\n", length);
    printf("  Secret width: %d meters\n", width);
    
    mpc_share_t shares_len[5], shares_wid[5];
    mpc_create_shares(&ctx, &length, shares_len);
    mpc_create_shares(&ctx, &width, shares_wid);
    
    // Calculate area securely
    mpc_share_t shares_area[5];
    mpc_secure_mul(&ctx, shares_len, shares_wid, shares_area, 5);
    
    uint8_t area;
    mpc_reconstruct(&ctx, shares_area, 3, &area);
    
    printf("  Computed area: %d square meters\n", area);
    printf("  Expected: 96 square meters\n");
    
    int success = (area == 96);
    
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares_len[i]);
        mpc_wipe_share(&shares_wid[i]);
        mpc_wipe_share(&shares_area[i]);
    }
    mpc_cleanup_context(&ctx);
    
    return success;
}

// Test 8: Different Share Subsets
int test_different_subsets() {
    printf("\n" COLOR_YELLOW "→ Test 8: Different Share Subsets" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t a = 7, b = 8;
    
    mpc_share_t shares_a[5], shares_b[5], shares_prod[5];
    mpc_create_shares(&ctx, &a, shares_a);
    mpc_create_shares(&ctx, &b, shares_b);
    mpc_secure_mul(&ctx, shares_a, shares_b, shares_prod, 5);
    
    // Test different subsets
    int subsets[][3] = {
        {0, 1, 2},  // Shares 1, 2, 3
        {0, 2, 4},  // Shares 1, 3, 5
        {1, 3, 4},  // Shares 2, 4, 5
        {2, 3, 4},  // Shares 3, 4, 5
    };
    
    int all_success = 1;
    
    for (int test = 0; test < 4; test++) {
        mpc_share_t subset[3];
        for (int i = 0; i < 3; i++) {
            subset[i] = shares_prod[subsets[test][i]];
        }
        
        uint8_t result;
        mpc_reconstruct(&ctx, subset, 3, &result);
        
        printf("  Subset {%d,%d,%d}: %d × %d = %d (expected 56)\n",
               subsets[test][0]+1, subsets[test][1]+1, subsets[test][2]+1,
               a, b, result);
        
        if (result != 56) {
            all_success = 0;
        }
    }
    
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares_a[i]);
        mpc_wipe_share(&shares_b[i]);
        mpc_wipe_share(&shares_prod[i]);
    }
    mpc_cleanup_context(&ctx);
    
    return all_success;
}

// Test 9: Error Handling - Insufficient Shares
int test_insufficient_shares() {
    printf("\n" COLOR_YELLOW "→ Test 9: Error Handling - Insufficient Shares" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t a = 10, b = 5;
    mpc_share_t shares_a[5], shares_b[5], shares_prod[5];
    
    mpc_create_shares(&ctx, &a, shares_a);
    mpc_create_shares(&ctx, &b, shares_b);
    
    // Try with only 2 shares (threshold is 3)
    int result = mpc_secure_mul(&ctx, shares_a, shares_b, shares_prod, 2);
    
    printf("  Attempted multiplication with 2 shares (threshold 3)\n");
    printf("  Expected: Failure\n");
    printf("  Actual: %s\n", result == -1 ? "Failure ✓" : "Success (ERROR!)");
    
    int success = (result == -1);
    
    mpc_cleanup_context(&ctx);
    return success;
}

int main() {
    printf("\n");
    printf(COLOR_MAGENTA "════════════════════════════════════════════════\n");
    printf("  MPC Secure Multiplication Tests\n");
    printf("  Week 5 - The Complex Operation!\n");
    printf("════════════════════════════════════════════════\n" COLOR_RESET);
    
    printf("\n" COLOR_CYAN "📚 Background:" COLOR_RESET "\n");
    printf("  Multiplication is challenging because:\n");
    printf("  • Multiplying polynomials increases degree\n");
    printf("  • Higher degree needs more shares\n");
    printf("  • Requires degree reduction protocol\n");
    printf("  • More complex than addition/subtraction\n");
    
    // Initialize
    printf("\n" COLOR_BLUE "→ Initializing library..." COLOR_RESET "\n");
    if (sss_init() != 0) {
        printf(COLOR_RED "✗ Failed to initialize\n" COLOR_RESET);
        return 1;
    }
    printf(COLOR_GREEN "✓ Library initialized\n" COLOR_RESET);
    
    // Run tests
    print_header("Basic Multiplication Tests");
    TEST_ASSERT(test_basic_multiplication(), "Basic Multiplication (5 × 6 = 30)");
    TEST_ASSERT(test_multiply_by_zero(), "Multiply by Zero (50 × 0 = 0)");
    TEST_ASSERT(test_multiply_by_one(), "Multiply by One (42 × 1 = 42)");
    TEST_ASSERT(test_large_numbers(), "Large Numbers in GF(256)");
    
    print_header("Advanced Tests");
    TEST_ASSERT(test_chained_multiplication(), "Chained Multiplication ((3×4)×5)");
    TEST_ASSERT(test_combined_operations(), "Combined Operations ((10×5)+20)");
    TEST_ASSERT(test_calculate_area(), "Real-World: Calculate Area");
    
    print_header("Robustness Tests");
    TEST_ASSERT(test_different_subsets(), "Different Share Subsets");
    TEST_ASSERT(test_insufficient_shares(), "Error: Insufficient Shares");
    
    // Summary
    printf("\n");
    print_separator();
    printf(COLOR_BLUE "  Test Summary\n" COLOR_RESET);
    print_separator();
    printf("\n");
    printf("  Total tests:   %d\n", tests_passed + tests_failed);
    printf("  " COLOR_GREEN "Passed:       %d\n" COLOR_RESET, tests_passed);
    printf("  " COLOR_RED "Failed:       %d\n" COLOR_RESET, tests_failed);
    printf("\n");
    print_separator();
    
    if (tests_failed == 0) {
        printf(COLOR_GREEN "\n  ✓✓✓ ALL TESTS PASSED! ✓✓✓\n" COLOR_RESET);
        printf(COLOR_GREEN "  Secure multiplication working perfectly!\n" COLOR_RESET);
        printf(COLOR_CYAN "\n  🎉 You now have FULL MPC arithmetic!\n" COLOR_RESET);
        printf(COLOR_CYAN "  • Addition ✓\n" COLOR_RESET);
        printf(COLOR_CYAN "  • Subtraction ✓\n" COLOR_RESET);
        printf(COLOR_CYAN "  • Multiplication by constant ✓\n" COLOR_RESET);
        printf(COLOR_CYAN "  • Multiplication of secrets ✓\n" COLOR_RESET);
        printf("\n");
        print_separator();
        return 0;
    } else {
        printf(COLOR_RED "\n  ✗✗✗ SOME TESTS FAILED ✗✗✗\n\n" COLOR_RESET);
        print_separator();
        return 1;
    }
}
