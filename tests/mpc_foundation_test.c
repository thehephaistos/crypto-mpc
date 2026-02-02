#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sodium.h>
#include "sss/mpc.h"
#include "sss/secret_sharing.h"

/* ========================================================================
 * ANSI Color Codes for Beautiful Output
 * ======================================================================== */
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_BOLD    "\x1b[1m"

/* ========================================================================
 * Test Result Tracking
 * ======================================================================== */
static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

/* ========================================================================
 * Helper Functions
 * ======================================================================== */

void print_header(const char *title) {
    printf("\n" COLOR_CYAN COLOR_BOLD);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  %s\n", title);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf(COLOR_RESET);
}

void print_test_result(const char *test_name, int passed) {
    total_tests++;
    if (passed) {
        printf("  " COLOR_GREEN "✓ PASS" COLOR_RESET ": %s\n", test_name);
        passed_tests++;
    } else {
        printf("  " COLOR_RED "✗ FAIL" COLOR_RESET ": %s\n", test_name);
        failed_tests++;
    }
}

/* ========================================================================
 * Test 1: Context Initialization Tests
 * ======================================================================== */
int test_context_initialization(void) {
    printf("\n" COLOR_YELLOW "→ Testing context initialization..." COLOR_RESET "\n");
    
    mpc_context_t ctx;
    int result;
    
    // Test 1.1: Valid context initialization
    result = mpc_init_context(&ctx, 5, 3, 1);
    int valid_init = (result == 0 && 
                      ctx.num_parties == 5 && 
                      ctx.threshold == 3 && 
                      ctx.value_size == 1);
    print_test_result("Context initialization with valid parameters", valid_init);
    
    // Test 1.2: Invalid parameters - NULL context
    result = mpc_init_context(NULL, 5, 3, 1);
    print_test_result("Reject NULL context", result == -1);
    
    // Test 1.3: Invalid parameters - too few parties
    result = mpc_init_context(&ctx, 1, 1, 1);
    print_test_result("Reject < 2 parties", result == -1);
    
    // Test 1.4: Invalid parameters - threshold > parties
    result = mpc_init_context(&ctx, 5, 6, 1);
    print_test_result("Reject threshold > num_parties", result == -1);
    
    // Test 1.5: Invalid parameters - zero value size
    result = mpc_init_context(&ctx, 5, 3, 0);
    print_test_result("Reject zero value_size", result == -1);
    
    // Test 1.6: Invalid parameters - threshold < 2
    result = mpc_init_context(&ctx, 5, 1, 1);
    print_test_result("Reject threshold < 2", result == -1);
    
    // Test 1.7: Edge case - minimum valid config
    result = mpc_init_context(&ctx, 2, 2, 1);
    int min_valid = (result == 0 && ctx.num_parties == 2 && ctx.threshold == 2);
    print_test_result("Minimum valid config (2 parties, threshold 2)", min_valid);
    
    return (failed_tests == 0) ? 0 : 1;
}

/* ========================================================================
 * Test 2: Context Cleanup Tests
 * ======================================================================== */
int test_context_cleanup(void) {
    printf("\n" COLOR_YELLOW "→ Testing context cleanup..." COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    // Cleanup
    mpc_cleanup_context(&ctx);
    
    // Test 2.1: Check that memory is wiped
    int memory_wiped = 1;
    uint8_t *ptr = (uint8_t *)&ctx;
    for (size_t i = 0; i < sizeof(mpc_context_t); i++) {
        if (ptr[i] != 0) {
            memory_wiped = 0;
            break;
        }
    }
    print_test_result("Context memory wiped after cleanup", memory_wiped);
    
    // Test 2.2: NULL cleanup should not crash
    mpc_cleanup_context(NULL);
    print_test_result("Cleanup with NULL context doesn't crash", 1);
    
    return (failed_tests == 0) ? 0 : 1;
}

/* ========================================================================
 * Test 3: Share Creation Tests
 * ======================================================================== */
int test_share_creation(void) {
    printf("\n" COLOR_YELLOW "→ Testing share creation..." COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t secret = 42;
    mpc_share_t shares[5];
    
    // Test 3.1: Create shares
    int result = mpc_create_shares(&ctx, &secret, shares);
    print_test_result("Share creation succeeds", result == 0);
    
    // Test 3.2: Verify correct number of shares
    int correct_count = 1;
    for (int i = 0; i < 5; i++) {
        if (shares[i].party_id != (i + 1)) {
            correct_count = 0;
            break;
        }
    }
    print_test_result("All shares have correct party_id (1-5)", correct_count);
    
    // Test 3.3: Verify computation_id is set
    int comp_id_set = 1;
    uint8_t expected_id = ctx.computation_id;
    for (int i = 0; i < 5; i++) {
        if (shares[i].computation_id != expected_id) {
            comp_id_set = 0;
            break;
        }
    }
    print_test_result("All shares have correct computation_id", comp_id_set);
    
    // Test 3.4: Verify data_len matches value_size
    int correct_len = 1;
    for (int i = 0; i < 5; i++) {
        if (shares[i].share.data_len != ctx.value_size) {
            correct_len = 0;
            break;
        }
    }
    print_test_result("All shares have correct data_len", correct_len);
    
    // Test 3.5: NULL parameter checks
    result = mpc_create_shares(NULL, &secret, shares);
    print_test_result("Reject NULL context in create_shares", result == -1);
    
    result = mpc_create_shares(&ctx, NULL, shares);
    print_test_result("Reject NULL secret in create_shares", result == -1);
    
    result = mpc_create_shares(&ctx, &secret, NULL);
    print_test_result("Reject NULL shares array in create_shares", result == -1);
    
    // Cleanup
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares[i]);
    }
    
    return 0;
}

/* ========================================================================
 * Test 4: Share Reconstruction Tests
 * ======================================================================== */
int test_share_reconstruction(void) {
    printf("\n" COLOR_YELLOW "→ Testing share reconstruction..." COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t secret = 123;
    mpc_share_t shares[5];
    mpc_create_shares(&ctx, &secret, shares);
    
    // Test 4.1: Reconstruct with threshold shares (first 3)
    uint8_t reconstructed = 0;
    int result = mpc_reconstruct(&ctx, shares, 3, &reconstructed);
    int first_combo = (result == 0 && reconstructed == secret);
    print_test_result("Reconstruct with shares 0,1,2", first_combo);
    
    // Test 4.2: Reconstruct with different combination (0,2,4)
    mpc_share_t combo2[3] = {shares[0], shares[2], shares[4]};
    reconstructed = 0;
    result = mpc_reconstruct(&ctx, combo2, 3, &reconstructed);
    int second_combo = (result == 0 && reconstructed == secret);
    print_test_result("Reconstruct with shares 0,2,4", second_combo);
    
    // Test 4.3: Reconstruct with different combination (1,3,4)
    mpc_share_t combo3[3] = {shares[1], shares[3], shares[4]};
    reconstructed = 0;
    result = mpc_reconstruct(&ctx, combo3, 3, &reconstructed);
    int third_combo = (result == 0 && reconstructed == secret);
    print_test_result("Reconstruct with shares 1,3,4", third_combo);
    
    // Test 4.4: Reconstruct with all shares
    reconstructed = 0;
    result = mpc_reconstruct(&ctx, shares, 5, &reconstructed);
    int all_shares = (result == 0 && reconstructed == secret);
    print_test_result("Reconstruct with all 5 shares", all_shares);
    
    // Test 4.5: Fail with too few shares
    reconstructed = 0;
    result = mpc_reconstruct(&ctx, shares, 2, &reconstructed);
    print_test_result("Reject reconstruction with < threshold shares", result == -1);
    
    // Test 4.6: NULL parameter checks
    result = mpc_reconstruct(NULL, shares, 3, &reconstructed);
    print_test_result("Reject NULL context in reconstruct", result == -1);
    
    result = mpc_reconstruct(&ctx, NULL, 3, &reconstructed);
    print_test_result("Reject NULL shares in reconstruct", result == -1);
    
    result = mpc_reconstruct(&ctx, shares, 3, NULL);
    print_test_result("Reject NULL output buffer in reconstruct", result == -1);
    
    // Cleanup
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares[i]);
    }
    
    return 0;
}

/* ========================================================================
 * Test 5: Share Validation Tests
 * ======================================================================== */
int test_share_validation(void) {
    printf("\n" COLOR_YELLOW "→ Testing share validation..." COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t secret = 77;
    mpc_share_t shares[5];
    mpc_create_shares(&ctx, &secret, shares);
    
    // Test 5.1: Validate correct shares
    int all_valid = 1;
    for (int i = 0; i < 5; i++) {
        if (mpc_validate_share(&ctx, &shares[i]) != 0) {
            all_valid = 0;
            break;
        }
    }
    print_test_result("All valid shares pass validation", all_valid);
    
    // Test 5.2: Invalid party_id (too high)
    mpc_share_t invalid_share = shares[0];
    invalid_share.party_id = 10;
    int result = mpc_validate_share(&ctx, &invalid_share);
    print_test_result("Reject share with party_id > num_parties", result == -1);
    
    // Test 5.3: Invalid party_id (zero)
    invalid_share = shares[0];
    invalid_share.party_id = 0;
    result = mpc_validate_share(&ctx, &invalid_share);
    print_test_result("Reject share with party_id = 0", result == -1);
    
    // Test 5.4: Wrong computation_id
    invalid_share = shares[0];
    invalid_share.computation_id = ctx.computation_id + 1;
    result = mpc_validate_share(&ctx, &invalid_share);
    print_test_result("Reject share with wrong computation_id", result == -1);
    
    // Test 5.5: Wrong data_len
    invalid_share = shares[0];
    invalid_share.share.data_len = ctx.value_size + 1;
    result = mpc_validate_share(&ctx, &invalid_share);
    print_test_result("Reject share with wrong data_len", result == -1);
    
    // Test 5.6: NULL parameter checks
    result = mpc_validate_share(NULL, &shares[0]);
    print_test_result("Reject NULL context in validate", result == -1);
    
    result = mpc_validate_share(&ctx, NULL);
    print_test_result("Reject NULL share in validate", result == -1);
    
    // Cleanup
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares[i]);
    }
    
    return 0;
}

/* ========================================================================
 * Test 6: Share Wiping Tests
 * ======================================================================== */
int test_share_wiping(void) {
    printf("\n" COLOR_YELLOW "→ Testing share wiping..." COLOR_RESET "\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t secret = 99;
    mpc_share_t shares[5];
    mpc_create_shares(&ctx, &secret, shares);
    
    // Test 6.1: Wipe a share
    mpc_wipe_share(&shares[0]);
    
    // Check if share is wiped
    int is_wiped = 1;
    uint8_t *ptr = (uint8_t *)&shares[0];
    for (size_t i = 0; i < sizeof(mpc_share_t); i++) {
        if (ptr[i] != 0) {
            is_wiped = 0;
            break;
        }
    }
    print_test_result("Share memory wiped after wipe_share", is_wiped);
    
    // Test 6.2: NULL wipe should not crash
    mpc_wipe_share(NULL);
    print_test_result("Wipe with NULL share doesn't crash", 1);
    
    // Cleanup remaining shares
    for (int i = 1; i < 5; i++) {
        mpc_wipe_share(&shares[i]);
    }
    
    return 0;
}

/* ========================================================================
 * Test 7: Edge Cases and Stress Tests
 * ======================================================================== */
int test_edge_cases(void) {
    printf("\n" COLOR_YELLOW "→ Testing edge cases..." COLOR_RESET "\n");
    
    // Test 7.1: Minimum configuration (2 parties, threshold 2)
    mpc_context_t min_ctx;
    int result = mpc_init_context(&min_ctx, 2, 2, 1);
    print_test_result("Initialize minimum config (2 parties, threshold 2)", result == 0);
    
    if (result == 0) {
        uint8_t secret = 55;
        mpc_share_t min_shares[2];
        result = mpc_create_shares(&min_ctx, &secret, min_shares);
        print_test_result("Create shares with minimum config", result == 0);
        
        if (result == 0) {
            uint8_t reconstructed = 0;
            result = mpc_reconstruct(&min_ctx, min_shares, 2, &reconstructed);
            int correct = (result == 0 && reconstructed == secret);
            print_test_result("Reconstruct with minimum config", correct);
            
            // Cleanup
            for (int i = 0; i < 2; i++) {
                mpc_wipe_share(&min_shares[i]);
            }
        }
        
        mpc_cleanup_context(&min_ctx);
    }
    
    // Test 7.2: Larger party count (10 parties)
    mpc_context_t large_ctx;
    result = mpc_init_context(&large_ctx, 10, 6, 1);
    print_test_result("Initialize with 10 parties", result == 0);
    
    if (result == 0) {
        uint8_t secret = 200;
        mpc_share_t *large_shares = malloc(10 * sizeof(mpc_share_t));
        if (large_shares != NULL) {
            result = mpc_create_shares(&large_ctx, &secret, large_shares);
            print_test_result("Create 10 shares", result == 0);
            
            if (result == 0) {
                uint8_t reconstructed = 0;
                result = mpc_reconstruct(&large_ctx, large_shares, 6, &reconstructed);
                int correct = (result == 0 && reconstructed == secret);
                print_test_result("Reconstruct from 6 of 10 shares", correct);
                
                // Cleanup
                for (int i = 0; i < 10; i++) {
                    mpc_wipe_share(&large_shares[i]);
                }
            }
            free(large_shares);
        }
        
        mpc_cleanup_context(&large_ctx);
    }
    
    // Test 7.3: Different value sizes
    mpc_context_t size_ctx;
    result = mpc_init_context(&size_ctx, 5, 3, 4);
    print_test_result("Initialize with 4-byte values", result == 0);
    
    if (result == 0) {
        uint8_t secret[4] = {0x12, 0x34, 0x56, 0x78};
        mpc_share_t shares[5];
        result = mpc_create_shares(&size_ctx, secret, shares);
        print_test_result("Create shares with 4-byte secret", result == 0);
        
        if (result == 0) {
            uint8_t reconstructed[4] = {0};
            result = mpc_reconstruct(&size_ctx, shares, 3, reconstructed);
            int correct = (result == 0 && 
                          memcmp(secret, reconstructed, 4) == 0);
            print_test_result("Reconstruct 4-byte secret", correct);
            
            // Cleanup
            for (int i = 0; i < 5; i++) {
                mpc_wipe_share(&shares[i]);
            }
        }
        
        mpc_cleanup_context(&size_ctx);
    }
    
    return 0;
}

/* ========================================================================
 * Main Test Runner
 * ======================================================================== */
int main(void) {
    // Print banner
    printf("\n" COLOR_BOLD);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  MPC Foundation Tests\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf(COLOR_RESET);
    
    // Initialize libsodium
    printf("\n" COLOR_YELLOW "→ Initializing library..." COLOR_RESET "\n");
    if (sodium_init() < 0) {
        printf(COLOR_RED "✗ Failed to initialize libsodium!" COLOR_RESET "\n");
        return 1;
    }
    printf(COLOR_GREEN "✓ Library initialized" COLOR_RESET "\n");
    
    // Run all test suites
    print_header("Context Management Tests");
    test_context_initialization();
    test_context_cleanup();
    
    print_header("Share Operations Tests");
    test_share_creation();
    test_share_reconstruction();
    
    print_header("Validation and Cleanup Tests");
    test_share_validation();
    test_share_wiping();
    
    print_header("Edge Cases and Stress Tests");
    test_edge_cases();
    
    // Print summary
    printf("\n" COLOR_BOLD);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Test Summary\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf(COLOR_RESET);
    
    printf("\n  " COLOR_BOLD "Total tests:   %d" COLOR_RESET "\n", total_tests);
    printf("  " COLOR_GREEN "Passed:       %d" COLOR_RESET "\n", passed_tests);
    
    if (failed_tests > 0) {
        printf("  " COLOR_RED "Failed:       %d" COLOR_RESET "\n", failed_tests);
    } else {
        printf("  Failed:       %d\n", failed_tests);
    }
    
    printf("\n");
    
    if (failed_tests == 0) {
        printf(COLOR_GREEN COLOR_BOLD "  ✓✓✓ ALL TESTS PASSED! ✓✓✓" COLOR_RESET "\n\n");
        return 0;
    } else {
        printf(COLOR_RED COLOR_BOLD "  ✗✗✗ SOME TESTS FAILED! ✗✗✗" COLOR_RESET "\n\n");
        return 1;
    }
}
