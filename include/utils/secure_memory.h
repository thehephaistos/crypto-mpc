#ifndef SSS_SECURE_MEMORY_H
#define SSS_SECURE_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Secure Memory Management using libsodium
 * 
 * This module provides memory protection features to prevent secrets
 * from being leaked through memory dumps, swap files, or core dumps.
 * ======================================================================== */

/**
 * Initialize the secure memory subsystem.
 * Must be called before using any other secure memory functions.
 * 
 * @return 0 on success, -1 on failure
 */
int secure_memory_init(void);

/**
 * Allocate and lock memory for sensitive data.
 * The allocated memory is: 
 * - Locked (won't be swapped to disk)
 * - Surrounded by guard pages (detect buffer overflows)
 * - Protected from core dumps
 * 
 * @param size  Number of bytes to allocate
 * @return Pointer to locked memory, or NULL on failure
 */
void* secure_malloc(size_t size);

/**
 * Securely free locked memory.
 * The memory is wiped before being freed.
 * 
 * @param ptr   Pointer to memory allocated with secure_malloc()
 * @param size  Size of the allocation
 */
void secure_free(void *ptr, size_t size);

/**
 * Lock existing memory to prevent swapping to disk.
 * Use this for stack-allocated or existing heap memory.
 * 
 * @param ptr   Pointer to memory to lock
 * @param size  Number of bytes to lock
 * @return 0 on success, -1 on failure
 */
int secure_lock(void *ptr, size_t size);

/**
 * Unlock previously locked memory.
 * Should be called before the memory goes out of scope.
 * 
 * @param ptr   Pointer to locked memory
 * @param size  Number of bytes to unlock
 * @return 0 on success, -1 on failure
 */
int secure_unlock(void *ptr, size_t size);

/**
 * Securely wipe memory. 
 * Unlike memset(), this cannot be optimized away by the compiler.
 * 
 * @param ptr   Pointer to memory to wipe
 * @param size  Number of bytes to wipe
 */
void secure_wipe(void *ptr, size_t size);

/**
 * Securely compare two memory regions in constant time.
 * Prevents timing attacks when comparing secrets.
 * 
 * @param ptr1  First memory region
 * @param ptr2  Second memory region
 * @param size  Number of bytes to compare
 * @return 0 if equal, -1 if different
 */
int secure_compare(const void *ptr1, const void *ptr2, size_t size);

/**
 * Check if two memory regions are equal (constant time).
 * Wrapper around secure_compare() that returns a boolean.
 * 
 * @param ptr1  First memory region
 * @param ptr2  Second memory region
 * @param size  Number of bytes to compare
 * @return 1 if equal, 0 if different
 */
int secure_is_equal(const void *ptr1, const void *ptr2, size_t size);

/**
 * Make memory region read-only.
 * Useful for protecting keys after generation.
 * 
 * @param ptr   Pointer to memory
 * @param size  Number of bytes
 * @return 0 on success, -1 on failure
 */
int secure_make_readonly(void *ptr, size_t size);

/**
 * Make memory region read-write again.
 * Use before modifying previously read-only memory.
 * 
 * @param ptr   Pointer to memory
 * @param size  Number of bytes
 * @return 0 on success, -1 on failure
 */
int secure_make_readwrite(void *ptr, size_t size);

/**
 * Make memory region inaccessible (no read/write).
 * Useful for creating guard pages or hiding sensitive data.
 * 
 * @param ptr   Pointer to memory
 * @param size  Number of bytes
 * @return 0 on success, -1 on failure
 */
int secure_make_noaccess(void *ptr, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* SSS_SECURE_MEMORY_H */
