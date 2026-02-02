#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "utils/secure_memory.h"
#include "sss/secret_sharing.h"

// ANSI color codes for output
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

// Helper function to print test results
void print_test_result(const char *test_name, int passed) {
    if (passed) {
        printf(ANSI_COLOR_GREEN "✓ PASS" ANSI_COLOR_RESET " - %s\n", test_name);
        tests_passed++;
    } else {
        printf(ANSI_COLOR_RED "✗ FAIL" ANSI_COLOR_RESET " - %s\n", test_name);
        tests_failed++;
    }
}

// Test 1: Initialization
int test_initialization(void) {
    printf("\n" ANSI_COLOR_YELLOW "Test 1: Initialization" ANSI_COLOR_RESET "\n");
    
    // Test secure_memory_init() returns 0
    int result = secure_memory_init();
    print_test_result("secure_memory_init() returns 0 (success)", result == SSS_OK);
    
    // Test calling it multiple times is safe
    result = secure_memory_init();
    print_test_result("Multiple init calls are safe", result == SSS_OK);
    
    return (result == SSS_OK) ? 0 : 1;
}

// Test 2: Memory Allocation
int test_memory_allocation(void) {
    printf("\n" ANSI_COLOR_YELLOW "Test 2: Memory Allocation" ANSI_COLOR_RESET "\n");
    
    // Allocate memory with secure_malloc()
    size_t size = 256;
    uint8_t *ptr = (uint8_t *)secure_malloc(size);
    print_test_result("secure_malloc() returns non-NULL", ptr != NULL);
    
    if (ptr == NULL) {
        return 1;
    }
    
    // Write to the memory
    memset(ptr, 0xAA, size);
    int write_success = (ptr[0] == 0xAA && ptr[size-1] == 0xAA);
    print_test_result("Can write to allocated memory", write_success);
    
    // Free with secure_free()
    secure_free(ptr, size);
    print_test_result("secure_free() completes without error", 1);
    
    return write_success ? 0 : 1;
}

// Test 3: Memory Locking
int test_memory_locking(void) {
    printf("\n" ANSI_COLOR_YELLOW "Test 3: Memory Locking" ANSI_COLOR_RESET "\n");
    
    // Create a stack variable
    uint8_t buffer[64];
    memset(buffer, 0, sizeof(buffer));
    
    // Lock it with secure_lock()
    int lock_result = secure_lock(buffer, sizeof(buffer));
    print_test_result("secure_lock() returns 0 (success)", lock_result == SSS_OK);
    
    // Unlock it with secure_unlock()
    int unlock_result = secure_unlock(buffer, sizeof(buffer));
    print_test_result("secure_unlock() returns 0 (success)", unlock_result == SSS_OK);
    
    // Test NULL pointer handling
    int null_lock = secure_lock(NULL, 100);
    print_test_result("secure_lock(NULL) returns error", null_lock == SSS_ERR_INVALID_PARAM);
    
    int null_unlock = secure_unlock(NULL, 100);
    print_test_result("secure_unlock(NULL) returns error", null_unlock == SSS_ERR_INVALID_PARAM);
    
    return (lock_result == SSS_OK && unlock_result == SSS_OK) ? 0 : 1;
}

// Test 4: Secure Wiping
int test_secure_wiping(void) {
    printf("\n" ANSI_COLOR_YELLOW "Test 4: Secure Wiping" ANSI_COLOR_RESET "\n");
    
    // Allocate memory, fill with data
    size_t size = 128;
    uint8_t *ptr = (uint8_t *)secure_malloc(size);
    
    if (ptr == NULL) {
        print_test_result("Memory allocation for wipe test", 0);
        return 1;
    }
    
    // Fill with non-zero data
    memset(ptr, 0xFF, size);
    int filled = (ptr[0] == 0xFF && ptr[size-1] == 0xFF);
    print_test_result("Memory filled with data (0xFF)", filled);
    
    // Wipe with secure_wipe()
    secure_wipe(ptr, size);
    
    // Verify memory is zeroed
    int all_zero = 1;
    for (size_t i = 0; i < size; i++) {
        if (ptr[i] != 0) {
            all_zero = 0;
            break;
        }
    }
    print_test_result("Memory wiped to zero", all_zero);
    
    // Test NULL pointer handling
    secure_wipe(NULL, 100); // Should not crash
    print_test_result("secure_wipe(NULL) doesn't crash", 1);
    
    secure_free(ptr, size);
    
    return all_zero ? 0 : 1;
}

// Test 5: Constant-Time Comparison
int test_constant_time_comparison(void) {
    printf("\n" ANSI_COLOR_YELLOW "Test 5: Constant-Time Comparison" ANSI_COLOR_RESET "\n");
    
    // Create two equal buffers
    uint8_t buf1[32];
    uint8_t buf2[32];
    memset(buf1, 0xAB, sizeof(buf1));
    memset(buf2, 0xAB, sizeof(buf2));
    
    // Test secure_compare() with equal data (should return 0)
    int cmp_equal = secure_compare(buf1, buf2, sizeof(buf1));
    print_test_result("secure_compare() returns 0 for equal data", cmp_equal == 0);
    
    // Make buf2 different
    buf2[15] = 0xCD;
    
    // Test secure_compare() with different data (should return -1)
    int cmp_diff = secure_compare(buf1, buf2, sizeof(buf1));
    print_test_result("secure_compare() returns -1 for different data", cmp_diff == -1);
    
    // Restore buf2 to match buf1
    buf2[15] = 0xAB;
    
    // Test secure_is_equal() with equal data (should return 1)
    int is_equal = secure_is_equal(buf1, buf2, sizeof(buf1));
    print_test_result("secure_is_equal() returns 1 for equal data", is_equal == 1);
    
    // Make buf2 different again
    buf2[20] = 0xEF;
    
    // Test secure_is_equal() with different data (should return 0)
    int is_not_equal = secure_is_equal(buf1, buf2, sizeof(buf1));
    print_test_result("secure_is_equal() returns 0 for different data", is_not_equal == 0);
    
    // Test NULL pointer handling
    int null_cmp = secure_compare(NULL, buf2, 32);
    print_test_result("secure_compare(NULL, ...) returns -1", null_cmp == -1);
    
    int null_eq = secure_is_equal(NULL, buf2, 32);
    print_test_result("secure_is_equal(NULL, ...) returns 0", null_eq == 0);
    
    return (cmp_equal == 0 && cmp_diff == -1 && is_equal == 1 && is_not_equal == 0) ? 0 : 1;
}

// Test 6: Memory Protection
int test_memory_protection(void) {
    printf("\n" ANSI_COLOR_YELLOW "Test 6: Memory Protection" ANSI_COLOR_RESET "\n");
    
    // Allocate memory with secure_malloc()
    size_t size = 256;
    uint8_t *ptr = (uint8_t *)secure_malloc(size);
    
    if (ptr == NULL) {
        print_test_result("Memory allocation for protection test", 0);
        return 1;
    }
    
    // Write some data
    memset(ptr, 0x42, size);
    print_test_result("Initial write to memory succeeds", ptr[0] == 0x42);
    
    // Make it read-only with secure_make_readonly()
    int readonly_result = secure_make_readonly(ptr, size);
    print_test_result("secure_make_readonly() returns 0", readonly_result == SSS_OK);
    
    // Verify we can read from it
    int can_read = (ptr[0] == 0x42 && ptr[size-1] == 0x42);
    print_test_result("Can read from read-only memory", can_read);
    
    // Note: We cannot safely test that writing fails without crashing the program
    // In production, writing to read-only memory would cause a segfault
    
    // Make it read-write again with secure_make_readwrite()
    int readwrite_result = secure_make_readwrite(ptr, size);
    print_test_result("secure_make_readwrite() returns 0", readwrite_result == SSS_OK);
    
    // Verify we can write to it again
    ptr[0] = 0x99;
    int can_write = (ptr[0] == 0x99);
    print_test_result("Can write after making read-write", can_write);
    
    // Test secure_make_noaccess() 
    int noaccess_result = secure_make_noaccess(ptr, size);
    print_test_result("secure_make_noaccess() returns 0", noaccess_result == SSS_OK);
    
    // Note: We cannot test accessing no-access memory without crashing
    
    // Make it read-write again to free it safely
    secure_make_readwrite(ptr, size);
    
    // Free the memory
    secure_free(ptr, size);
    print_test_result("Cleanup after protection tests", 1);
    
    return (readonly_result == SSS_OK && readwrite_result == SSS_OK && 
            noaccess_result == SSS_OK && can_read && can_write) ? 0 : 1;
}

int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("  Secure Memory Management Test Suite  \n");
    printf("========================================\n");
    
    // Initialize the library
    if (sodium_init() < 0) {
        printf(ANSI_COLOR_RED "Failed to initialize libsodium!" ANSI_COLOR_RESET "\n");
        return 1;
    }
    
    // Run all tests
    int result = 0;
    result |= test_initialization();
    result |= test_memory_allocation();
    result |= test_memory_locking();
    result |= test_secure_wiping();
    result |= test_constant_time_comparison();
    result |= test_memory_protection();
    
    // Print summary
    printf("\n========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n");
    printf(ANSI_COLOR_GREEN "Passed: %d" ANSI_COLOR_RESET "\n", tests_passed);
    
    if (tests_failed > 0) {
        printf(ANSI_COLOR_RED "Failed: %d" ANSI_COLOR_RESET "\n", tests_failed);
    } else {
        printf("Failed: %d\n", tests_failed);
    }
    
    printf("========================================\n\n");
    
    if (result == 0 && tests_failed == 0) {
        printf(ANSI_COLOR_GREEN "✓ All tests passed!" ANSI_COLOR_RESET "\n\n");
        return 0;
    } else {
        printf(ANSI_COLOR_RED "✗ Some tests failed!" ANSI_COLOR_RESET "\n\n");
        return 1;
    }
}
