#ifndef SSS_UTILS_RANDOM_H
#define SSS_UTILS_RANDOM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Random Number Generation
 * ======================================================================== */

/**
 * Generate random bytes using libsodium
 * 
 * @param buffer Buffer to fill with random bytes
 * @param length Number of random bytes to generate
 * 
 * @return 0 on success, -1 on error
 */
int sss_random_bytes(uint8_t *buffer, size_t length);

/**
 * Generate a random byte in GF(256) (non-zero)
 * 
 * @return Random byte in range [1, 255]
 * 
 * Used for polynomial coefficients (cannot be zero for security)
 */
uint8_t sss_random_nonzero(void);

/**
 * Generate a random byte in GF(256) (any value)
 * 
 * @return Random byte in range [0, 255]
 */
uint8_t sss_random_byte(void);

#ifdef __cplusplus
}
#endif

#endif /* SSS_UTILS_RANDOM_H */