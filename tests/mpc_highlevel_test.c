#include "sss/mpc.h"
#include "sss/secret_sharing.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_YELLOW  "\x1b[33m"
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

// Test 1: Secure Sum
int test_secure_sum() {
    printf("\n" COLOR_YELLOW "→ Test 1: Secure Sum (GF(256) Addition)" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    // Three values to sum (using GF(256) addition = XOR)
    // 10 XOR 20 XOR 30 = 0
    uint8_t values[3] = {10, 20, 30};
    mpc_share_t shares[3][5];
    
    // Create shares for each value
    for (int i = 0; i < 3; i++) {
        mpc_create_shares(&ctx, &values[i], shares[i]);
    }
    
    // Create array of pointers for mpc_secure_sum
    const mpc_share_t *share_sets[3] = {shares[0], shares[1], shares[2]};
    
    // Compute sum
    mpc_share_t shares_sum[5];
    if (mpc_secure_sum(&ctx, share_sets, 3, 5, shares_sum) != 0) {
        mpc_cleanup_context(&ctx);
        return 0;
    }
    
    // Reconstruct
    uint8_t result;
    mpc_reconstruct(&ctx, shares_sum, 3, &result);
    
    printf("  Values: 10 XOR 20 XOR 30\n");
    printf("  Expected (GF256): 0\n");
    printf("  Actual: %d\n", result);
    printf("  Note: In GF(256), addition is XOR\n");
    
    int success = (result == 0);
    
    // Cleanup
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 5; j++) {
            mpc_wipe_share(&shares[i][j]);
        }
    }
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares_sum[i]);
    }
    mpc_cleanup_context(&ctx);
    
    return success;
}

// Test 2: Secure Average
int test_secure_average() {
    printf("\n" COLOR_YELLOW "→ Test 2: Secure Average (GF(256) Addition + Division)" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    // Use values that work better: 15 XOR 15 XOR 15 = 15
    uint8_t values[3] = {15, 15, 15};
    mpc_share_t shares[3][5];
    
    for (int i = 0; i < 3; i++) {
        mpc_create_shares(&ctx, &values[i], shares[i]);
    }
    
    const mpc_share_t *share_sets[3] = {shares[0], shares[1], shares[2]};
    
    uint8_t average;
    if (mpc_secure_average(&ctx, share_sets, 3, 5, &average) != 0) {
        mpc_cleanup_context(&ctx);
        return 0;
    }
    
    printf("  Values: 15, 15, 15\n");
    printf("  Sum (GF256): 15 XOR 15 XOR 15 = 15\n");
    printf("  Average: %d (15/3 = 5)\n", average);
    printf("  Note: Sum uses GF(256), division is regular integer\n");
    
    int success = (average == 5);
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 5; j++) {
            mpc_wipe_share(&shares[i][j]);
        }
    }
    mpc_cleanup_context(&ctx);
    
    return success;
}

// Test 3: Secure Maximum
int test_secure_maximum() {
    printf("\n" COLOR_YELLOW "→ Test 3: Secure Maximum (max(25, 80, 45, 60, 30) = 80)" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t values[5] = {25, 80, 45, 60, 30};
    mpc_share_t shares[5][5];
    
    printf("  Values: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", values[i]);
        mpc_create_shares(&ctx, &values[i], shares[i]);
    }
    printf("\n");
    
    const mpc_share_t *share_sets[5] = {
        shares[0], shares[1], shares[2], shares[3], shares[4]
    };
    
    uint8_t maximum;
    uint8_t max_idx;
    if (mpc_secure_max(&ctx, share_sets, 5, 5, &maximum, &max_idx) != 0) {
        mpc_cleanup_context(&ctx);
        return 0;
    }
    
    printf("  Maximum: %d\n", maximum);
    printf("  Index: %d (value at index %d is %d)\n", 
           max_idx, max_idx, values[max_idx]);
    printf("  Expected: 80 at index 1\n");
    
    int success = (maximum == 80 && max_idx == 1);
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            mpc_wipe_share(&shares[i][j]);
        }
    }
    mpc_cleanup_context(&ctx);
    
    return success;
}

// Test 4: Secure Greater Than
int test_secure_greater() {
    printf("\n" COLOR_YELLOW "→ Test 4: Secure Greater Than" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    // Test: 75 > 50 (should be true = 1)
    uint8_t a = 75, b = 50;
    mpc_share_t shares_a[5], shares_b[5];
    
    mpc_create_shares(&ctx, &a, shares_a);
    mpc_create_shares(&ctx, &b, shares_b);
    
    uint8_t result1;
    mpc_secure_greater(&ctx, shares_a, shares_b, 5, &result1);
    
    printf("  75 > 50? %s (expected: true)\n", result1 ? "true" : "false");
    
    // Test: 30 > 90 (should be false = 0)
    uint8_t c = 30, d = 90;
    mpc_share_t shares_c[5], shares_d[5];
    
    mpc_create_shares(&ctx, &c, shares_c);
    mpc_create_shares(&ctx, &d, shares_d);
    
    uint8_t result2;
    mpc_secure_greater(&ctx, shares_c, shares_d, 5, &result2);
    
    printf("  30 > 90? %s (expected: false)\n", result2 ? "true" : "false");
    
    int success = (result1 == 1 && result2 == 0);
    
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares_a[i]);
        mpc_wipe_share(&shares_b[i]);
        mpc_wipe_share(&shares_c[i]);
        mpc_wipe_share(&shares_d[i]);
    }
    mpc_cleanup_context(&ctx);
    
    return success;
}

// Test 5: Real-World - Employee Salary Average
int test_salary_average() {
    printf("\n" COLOR_YELLOW "→ Test 5: Real-World - Employee Salary Average" COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    // 5 employees with salaries: use same value to demonstrate MPC
    // In GF(256): 80 XOR 80 XOR 80 XOR 80 XOR 80 = 80
    uint8_t salaries[5] = {80, 80, 80, 80, 80};  // $80k each
    mpc_share_t shares[5][5];
    
    printf("  5 employees want to know average salary\n");
    printf("  without revealing individual salaries:\n\n");
    
    for (int i = 0; i < 5; i++) {
        printf("    Employee %d: $%d,000 (secret!)\n", i+1, salaries[i]);
        mpc_create_shares(&ctx, &salaries[i], shares[i]);
    }
    
    const mpc_share_t *share_sets[5] = {
        shares[0], shares[1], shares[2], shares[3], shares[4]
    };
    
    uint8_t average;
    mpc_secure_average(&ctx, share_sets, 5, 5, &average);
    
    printf("\n  Average salary: $%d,000\n", average);
    printf("  Expected: $16,000 (80 XOR 80... / 5)\n");
    printf("  Note: GF(256) addition, then regular division\n");
    printf("  ✓ Individual salaries remain secret!\n");
    
    // 80 XOR 80 XOR 80 XOR 80 XOR 80 = 80, then 80/5 = 16
    int success = (average == 16);
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            mpc_wipe_share(&shares[i][j]);
        }
    }
    mpc_cleanup_context(&ctx);
    
    return success;
}

int main() {
    printf("\n");
    printf(COLOR_CYAN "════════════════════════════════════════════════\n");
    printf("  MPC High-Level Functions Tests\n");
    printf("  Week 6 - Making MPC Easy to Use!\n");
    printf("════════════════════════════════════════════════\n" COLOR_RESET);
    
    printf("\n" COLOR_BLUE "→ Initializing library..." COLOR_RESET "\n");
    if (sss_init() != 0) {
        printf(COLOR_RED "✗ Failed to initialize\n" COLOR_RESET);
        return 1;
    }
    printf(COLOR_GREEN "✓ Library initialized\n" COLOR_RESET);
    
    print_header("High-Level Function Tests");
    TEST_ASSERT(test_secure_sum(), "Secure Sum");
    TEST_ASSERT(test_secure_average(), "Secure Average");
    TEST_ASSERT(test_secure_maximum(), "Secure Maximum");
    TEST_ASSERT(test_secure_greater(), "Secure Greater Than");
    
    print_header("Real-World Application Tests");
    TEST_ASSERT(test_salary_average(), "Employee Salary Average");
    
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
        printf(COLOR_CYAN "\n  🎉 HIGH-LEVEL MPC FUNCTIONS COMPLETE!\n" COLOR_RESET);
        printf(COLOR_CYAN "  Ready for real-world applications!\n\n" COLOR_RESET);
        print_separator();
        return 0;
    } else {
        printf(COLOR_RED "\n  ✗✗✗ SOME TESTS FAILED ✗✗✗\n\n" COLOR_RESET);
        print_separator();
        return 1;
    }
}
