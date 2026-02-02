#include "utils/secure_memory.h"
#include "utils/error.h"
#include <sodium.h>
#include <string.h>
#include <stdlib.h>

// Track whether libsodium has been initialized
static int is_initialized = 0;

// Initialize the secure memory subsystem
int secure_memory_init(void) {
    if (is_initialized) {
        return SSS_OK; // Already initialized
    }
    
    if (sodium_init() < 0) {
        return SSS_ERR_MEMORY; // Fail if libsodium can't initialize
    }
    
    is_initialized = 1;
    return SSS_OK;
}

// Allocate locked memory that won't be swapped to disk
// Uses sodium_malloc which adds guard pages and canaries
void* secure_malloc(size_t size) {
    if (size == 0) return NULL;
    
    // Ensure libsodium is initialized
    if (!is_initialized) {
        secure_memory_init();
    }
    
    // sodium_malloc guarantees the memory is locked and zeroed
    void *ptr = sodium_malloc(size);
    
    if (ptr == NULL) {
        return NULL;
    }
    
    return ptr;
}

// Securely wipe and free memory
void secure_free(void *ptr, size_t size) {
    if (ptr == NULL) return;

    // Explicitly wipe the memory first
    // sodium_memzero is robust against compiler optimizations removing it
    sodium_memzero(ptr, size);
    
    // sodium_free() also automatically wipes memory before freeing
    sodium_free(ptr);
}

// Prevent memory from being swapped to disk (locking)
int secure_lock(void *ptr, size_t size) {
    if (ptr == NULL || size == 0) return SSS_ERR_INVALID_PARAM;
    
    // Ensure libsodium is initialized
    if (!is_initialized) {
        secure_memory_init();
    }
    
    // sodium_mlock wraps mlock() and handles limits correctly
    if (sodium_mlock(ptr, size) != 0) {
        return SSS_ERR_MEMORY;
    }
    return SSS_OK;
}

// Allow memory to be swapped again
int secure_unlock(void *ptr, size_t size) {
    if (ptr == NULL || size == 0) return SSS_ERR_INVALID_PARAM;
    
    if (sodium_munlock(ptr, size) != 0) {
        return SSS_ERR_MEMORY;
    }
    return SSS_OK;
}

// Securely zero out memory immediately
void secure_wipe(void *ptr, size_t size) {
    if (ptr == NULL || size == 0) return;
    
    // Ensure libsodium is initialized
    if (!is_initialized) {
        secure_memory_init();
    }
    
    sodium_memzero(ptr, size);
}

// Constant-time comparison to prevent timing attacks
// Returns 0 if equal, -1 otherwise
int secure_compare(const void *ptr1, const void *ptr2, size_t size) {
    if (ptr1 == NULL || ptr2 == NULL) return -1;
    
    // Ensure libsodium is initialized
    if (!is_initialized) {
        secure_memory_init();
    }
    
    // sodium_memcmp returns 0 if match, -1 if mismatch
    // It takes constant time regardless of where the mismatch occurs
    return sodium_memcmp(ptr1, ptr2, size);
}

// Boolean wrapper for secure_compare
// Returns 1 if equal, 0 if different
int secure_is_equal(const void *ptr1, const void *ptr2, size_t size) {
    if (ptr1 == NULL || ptr2 == NULL) return 0;
    
    // Return 1 if secure_compare returns 0 (equal), otherwise return 0
    return (secure_compare(ptr1, ptr2, size) == 0) ? 1 : 0;
}

// Make memory read-only (protected)
int secure_make_readonly(void *ptr, size_t size) {
    if (ptr == NULL) return SSS_ERR_INVALID_PARAM;
    
    // Note: sodium_mprotect functions don't use size parameter
    // They work with memory allocated by sodium_malloc which knows its own size
    (void)size; // Suppress unused parameter warning
    
    if (sodium_mprotect_readonly(ptr) != 0) {
        return SSS_ERR_MEMORY;
    }
    return SSS_OK;
}

// Make memory read-write again
int secure_make_readwrite(void *ptr, size_t size) {
    if (ptr == NULL) return SSS_ERR_INVALID_PARAM;
    
    // Note: sodium_mprotect functions don't use size parameter
    (void)size; // Suppress unused parameter warning
    
    if (sodium_mprotect_readwrite(ptr) != 0) {
        return SSS_ERR_MEMORY;
    }
    return SSS_OK;
}

// Remove all access to memory (useful for keys not currently in use)
int secure_make_noaccess(void *ptr, size_t size) {
    if (ptr == NULL) return SSS_ERR_INVALID_PARAM;
    
    // Note: sodium_mprotect functions don't use size parameter
    (void)size; // Suppress unused parameter warning
    
    if (sodium_mprotect_noaccess(ptr) != 0) {
        return SSS_ERR_MEMORY;
    }
    return SSS_OK;
}
