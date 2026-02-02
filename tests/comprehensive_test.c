#include "sss/secret_sharing.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* ANSI color codes */
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

/* Test statistics */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* Helper to print test results */
void print_test_result(const char *test_name, bool passed) {
    tests_run++;
    if (passed) {
        tests_passed++;
        printf(COLOR_GREEN "  ✓ PASS:  %s\n" COLOR_RESET, test_name);
    } else {
        tests_failed++;
        printf(COLOR_RED "  ✗ FAIL: %s\n" COLOR_RESET, test_name);
    }
}

/* Helper to print section headers */
void print_section(const char *title) {
    printf("\n");
    printf(COLOR_CYAN "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" COLOR_RESET);
    printf(COLOR_CYAN "  %s\n" COLOR_RESET, title);
    printf(COLOR_CYAN "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" COLOR_RESET);
}

/* Test 1: Basic functionality with different thresholds */
bool test_basic_thresholds(void) {
    const uint8_t secret[] = "Test Secret 123";
    size_t secret_len = strlen((char*)secret);
    
    /* Test threshold=2, shares=3 */
    sss_share_t shares[3];
    int result = sss_create_shares(secret, secret_len, 2, 3, shares);
    if (result != SSS_OK) return false;
    
    uint8_t reconstructed[SSS_MAX_SECRET_SIZE];
    size_t reconstructed_len = sizeof(reconstructed);
    
    sss_share_t subset[2] = {shares[0], shares[2]};
    result = sss_combine_shares(subset, 2, reconstructed, &reconstructed_len);
    if (result != SSS_OK) return false;
    
    bool match = (reconstructed_len == secret_len && 
                  memcmp(secret, reconstructed, secret_len) == 0);
    
    /* Cleanup */
    for (int i = 0; i < 3; i++) sss_wipe_share(&shares[i]);
    
    return match;
}

/* Test 2: Minimum threshold (2-of-2) */
bool test_minimum_threshold(void) {
    const uint8_t secret[] = "Minimum! ";
    size_t secret_len = strlen((char*)secret);
    
    sss_share_t shares[2];
    int result = sss_create_shares(secret, secret_len, 2, 2, shares);
    if (result != SSS_OK) return false;
    
    uint8_t reconstructed[SSS_MAX_SECRET_SIZE];
    size_t reconstructed_len = sizeof(reconstructed);
    
    result = sss_combine_shares(shares, 2, reconstructed, &reconstructed_len);
    if (result != SSS_OK) return false;
    
    bool match = (reconstructed_len == secret_len && 
                  memcmp(secret, reconstructed, secret_len) == 0);
    
    /* Cleanup */
    for (int i = 0; i < 2; i++) sss_wipe_share(&shares[i]);
    
    return match;
}

/* Test 3: Using all shares (not just threshold) */
bool test_all_shares(void) {
    const uint8_t secret[] = "Use All Shares!";
    size_t secret_len = strlen((char*)secret);
    
    sss_share_t shares[5];
    int result = sss_create_shares(secret, secret_len, 3, 5, shares);
    if (result != SSS_OK) return false;
    
    uint8_t reconstructed[SSS_MAX_SECRET_SIZE];
    size_t reconstructed_len = sizeof(reconstructed);
    
    /* Use all 5 shares instead of just 3 */
    result = sss_combine_shares(shares, 5, reconstructed, &reconstructed_len);
    if (result != SSS_OK) return false;
    
    bool match = (reconstructed_len == secret_len && 
                  memcmp(secret, reconstructed, secret_len) == 0);
    
    /* Cleanup */
    for (int i = 0; i < 5; i++) sss_wipe_share(&shares[i]);
    
    return match;
}

/* Test 4: Different share combinations */
bool test_different_combinations(void) {
    const uint8_t secret[] = "Combination Test";
    size_t secret_len = strlen((char*)secret);
    
    sss_share_t shares[5];
    int result = sss_create_shares(secret, secret_len, 3, 5, shares);
    if (result != SSS_OK) return false;
    
    /* Try combination 1: shares 0, 1, 2 */
    uint8_t reconstructed[SSS_MAX_SECRET_SIZE];
    size_t reconstructed_len = sizeof(reconstructed);
    sss_share_t combo1[3] = {shares[0], shares[1], shares[2]};
    result = sss_combine_shares(combo1, 3, reconstructed, &reconstructed_len);
    if (result != SSS_OK || reconstructed_len != secret_len || 
        memcmp(secret, reconstructed, secret_len) != 0) {
        return false;
    }
    
    /* Try combination 2: shares 1, 3, 4 */
    reconstructed_len = sizeof(reconstructed);
    sss_share_t combo2[3] = {shares[1], shares[3], shares[4]};
    result = sss_combine_shares(combo2, 3, reconstructed, &reconstructed_len);
    if (result != SSS_OK || reconstructed_len != secret_len || 
        memcmp(secret, reconstructed, secret_len) != 0) {
        return false;
    }
    
    /* Try combination 3: shares 0, 2, 4 */
    reconstructed_len = sizeof(reconstructed);
    sss_share_t combo3[3] = {shares[0], shares[2], shares[4]};
    result = sss_combine_shares(combo3, 3, reconstructed, &reconstructed_len);
    if (result != SSS_OK || reconstructed_len != secret_len || 
        memcmp(secret, reconstructed, secret_len) != 0) {
        return false;
    }
    
    /* Cleanup */
    for (int i = 0; i < 5; i++) sss_wipe_share(&shares[i]);
    
    return true;
}

/* Test 5: Large secret (32-byte encryption key) */
bool test_large_secret(void) {
    /* Simulate a 256-bit (32-byte) encryption key */
    uint8_t secret[32];
    for (int i = 0; i < 32; i++) {
        secret[i] = (uint8_t)i;
    }
    
    sss_share_t shares[7];
    int result = sss_create_shares(secret, 32, 4, 7, shares);
    if (result != SSS_OK) return false;
    
    uint8_t reconstructed[SSS_MAX_SECRET_SIZE];
    size_t reconstructed_len = sizeof(reconstructed);
    
    sss_share_t subset[4] = {shares[1], shares[3], shares[4], shares[6]};
    result = sss_combine_shares(subset, 4, reconstructed, &reconstructed_len);
    if (result != SSS_OK) return false;
    
    bool match = (reconstructed_len == 32 && 
                  memcmp(secret, reconstructed, 32) == 0);
    
    /* Cleanup */
    for (int i = 0; i < 7; i++) sss_wipe_share(&shares[i]);
    
    return match;
}

/* Test 6: Too few shares should fail */
bool test_too_few_shares(void) {
    const uint8_t secret[] = "Need More Shares!";
    size_t secret_len = strlen((char*)secret);
    
    sss_share_t shares[5];
    int result = sss_create_shares(secret, secret_len, 4, 5, shares);
    if (result != SSS_OK) return false;
    
    uint8_t reconstructed[SSS_MAX_SECRET_SIZE];
    size_t reconstructed_len = sizeof(reconstructed);
    
    /* Try to reconstruct with only 3 shares (need 4) */
    sss_share_t subset[3] = {shares[0], shares[2], shares[4]};
    result = sss_combine_shares(subset, 3, reconstructed, &reconstructed_len);
    
    /* Cleanup */
    for (int i = 0; i < 5; i++) sss_wipe_share(&shares[i]);
    
    /* This should fail!  */
    return (result != SSS_OK);
}

/* Test 7: Invalid parameters */
bool test_invalid_parameters(void) {
    const uint8_t secret[] = "Test";
    sss_share_t shares[5];
    
    /* Threshold > num_shares (invalid) */
    int result1 = sss_create_shares(secret, 4, 6, 5, shares);
    
    /* Threshold = 1 (invalid, minimum is 2) */
    int result2 = sss_create_shares(secret, 4, 1, 5, shares);
    
    /* Zero shares (invalid) */
    int result3 = sss_create_shares(secret, 4, 2, 0, shares);
    
    /* NULL secret */
    int result4 = sss_create_shares(NULL, 4, 2, 5, shares);
    
    /* All should fail */
    return (result1 != SSS_OK && result2 != SSS_OK && 
            result3 != SSS_OK && result4 != SSS_OK);
}

/* Test 8: Empty secret */
bool test_empty_secret(void) {
    const uint8_t secret[] = "";
    
    sss_share_t shares[3];
    int result = sss_create_shares(secret, 0, 2, 3, shares);
    
    /* Should fail - can't split empty secret */
    return (result != SSS_OK);
}

/* Test 9: Maximum shares (255) */
bool test_maximum_shares(void) {
    const uint8_t secret[] = "Max shares! ";
    size_t secret_len = strlen((char*)secret);
    
    /* Allocate 255 shares (maximum possible in GF(256)) */
    sss_share_t *shares = malloc(255 * sizeof(sss_share_t));
    if (! shares) return false;
    
    int result = sss_create_shares(secret, secret_len, 128, 255, shares);
    if (result != SSS_OK) {
        free(shares);
        return false;
    }
    
    /* Reconstruct using exactly 128 shares */
    uint8_t reconstructed[SSS_MAX_SECRET_SIZE];
    size_t reconstructed_len = sizeof(reconstructed);
    
    result = sss_combine_shares(shares, 128, reconstructed, &reconstructed_len);
    
    bool match = (result == SSS_OK && reconstructed_len == secret_len && 
                  memcmp(secret, reconstructed, secret_len) == 0);
    
    /* Cleanup */
    for (int i = 0; i < 255; i++) sss_wipe_share(&shares[i]);
    free(shares);
    
    return match;
}

/* Test 10: Single byte secret */
bool test_single_byte_secret(void) {
    const uint8_t secret[] = {0x42};  /* Just one byte */
    
    sss_share_t shares[3];
    int result = sss_create_shares(secret, 1, 2, 3, shares);
    if (result != SSS_OK) return false;
    
    uint8_t reconstructed[SSS_MAX_SECRET_SIZE];
    size_t reconstructed_len = sizeof(reconstructed);
    
    sss_share_t subset[2] = {shares[0], shares[2]};
    result = sss_combine_shares(subset, 2, reconstructed, &reconstructed_len);
    if (result != SSS_OK) return false;
    
    bool match = (reconstructed_len == 1 && reconstructed[0] == 0x42);
    
    /* Cleanup */
    for (int i = 0; i < 3; i++) sss_wipe_share(&shares[i]);
    
    return match;
}

/* Main test runner */
int main(void) {
    printf("\n");
    printf(COLOR_BLUE "════════════════════════════════════════════════\n" COLOR_RESET);
    printf(COLOR_BLUE "  Shamir's Secret Sharing - Comprehensive Tests\n" COLOR_RESET);
    printf(COLOR_BLUE "═════════════════════��══════════════════════════\n" COLOR_RESET);
    
    /* Initialize library */
    printf("\n");
    printf(COLOR_YELLOW "→ Initializing library...\n" COLOR_RESET);
    int result = sss_init();
    if (result != SSS_OK) {
        printf(COLOR_RED "✗ Initialization failed: %s\n" COLOR_RESET, sss_strerror(result));
        return 1;
    }
    printf(COLOR_GREEN "✓ Library initialized\n" COLOR_RESET);
    
    /* Run tests */
    print_section("Basic Functionality Tests");
    print_test_result("Basic threshold test (3-of-5)", test_basic_thresholds());
    print_test_result("Minimum threshold (2-of-2)", test_minimum_threshold());
    print_test_result("Using all shares (5-of-5 with threshold 3)", test_all_shares());
    
    print_section("Share Combination Tests");
    print_test_result("Different share combinations", test_different_combinations());
    
    print_section("Secret Size Tests");
    print_test_result("Large secret (32 bytes)", test_large_secret());
    print_test_result("Single byte secret", test_single_byte_secret());
    
    print_section("Error Handling Tests");
    print_test_result("Too few shares (should fail)", test_too_few_shares());
    print_test_result("Invalid parameters (should fail)", test_invalid_parameters());
    print_test_result("Empty secret (should fail)", test_empty_secret());
    
    print_section("Edge Case Tests");
    print_test_result("Maximum shares (255)", test_maximum_shares());
    
    /* Print summary */
    printf("\n");
    printf(COLOR_BLUE "════════════════════════════════════════════════\n" COLOR_RESET);
    printf(COLOR_BLUE "  Test Summary\n" COLOR_RESET);
    printf(COLOR_BLUE "════════════════════════════════════════════════\n" COLOR_RESET);
    printf("\n");
    printf("  Total tests:   %d\n", tests_run);
    printf(COLOR_GREEN "  Passed:       %d\n" COLOR_RESET, tests_passed);
    if (tests_failed > 0) {
        printf(COLOR_RED "  Failed:        %d\n" COLOR_RESET, tests_failed);
    } else {
        printf("  Failed:       %d\n", tests_failed);
    }
    printf("\n");
    
    if (tests_failed == 0) {
        printf(COLOR_GREEN "  ✓✓✓ ALL TESTS PASSED! ✓✓✓\n" COLOR_RESET);
        printf(COLOR_GREEN "  Your secret sharing library is working perfectly!\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "  ✗✗✗ SOME TESTS FAILED ✗✗✗\n" COLOR_RESET);
        printf(COLOR_RED "  Please review the failures above.\n" COLOR_RESET);
    }
    
    printf("\n");
    printf(COLOR_BLUE "════════════════════════════════════════════════\n" COLOR_RESET);
    printf("\n");
    
    return (tests_failed == 0) ? 0 : 1;
}
