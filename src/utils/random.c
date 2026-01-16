#include "random.h"
#include <sodium.h>

/* ========================================================================
 * Random Bytes Generation
 * ======================================================================== */

/**
 * Generate cryptographically secure random bytes
 * 
 * Uses libsodium's randombytes_buf() which is: 
 * - Cryptographically secure
 * - Thread-safe
 * - Properly seeded
 */
int sss_random_bytes(uint8_t *buffer, size_t length) {
    if (buffer == NULL || length == 0) {
        return -1;
    }
    
    /* Generate random bytes using libsodium */
    randombytes_buf(buffer, length);
    
    return 0;
}

/* ========================================================================
 * Generate Random Non-Zero Byte
 * ======================================================================== */

/**
 * Generate a random non-zero byte in GF(256)
 * 
 * Used for polynomial coefficients - zero coefficients would
 * reduce the polynomial degree and weaken security. 
 */
uint8_t sss_random_nonzero(void) {
    uint8_t value;
    
    /* Keep generating until we get a non-zero value */
    do {
        randombytes_buf(&value, 1);
    } while (value == 0);
    
    return value;
}

/* ========================================================================
 * Generate Random Byte (Any Value)
 * ======================================================================== */

/**
 * Generate a random byte (including zero)
 */
uint8_t sss_random_byte(void) {
    uint8_t value;
    randombytes_buf(&value, 1);
    return value;
}