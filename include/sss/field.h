#ifndef SSS_FIELD_H
#define SSS_FIELD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Galois Field GF(256) Operations
 * ======================================================================== */

/**
 * GF(256) is a finite field with 256 elements (0-255)
 * 
 * We use the irreducible polynomial: 
 *   x⁸ + x⁴ + x³ + x + 1  (0x11B in hex)
 * 
 * This is the same polynomial used in AES
 */

/**
 * Add two elements in GF(256)
 * 
 * @param a First element
 * @param b Second element
 * @return a + b in GF(256)
 * 
 * In GF(256), addition is XOR
 */
static inline uint8_t gf256_add(uint8_t a, uint8_t b) {
    return a ^ b;
}

/**
 * Subtract two elements in GF(256)
 * 
 * @param a First element
 * @param b Second element
 * @return a - b in GF(256)
 * 
 * In GF(256), subtraction is also XOR (same as addition)
 */
static inline uint8_t gf256_sub(uint8_t a, uint8_t b) {
    return a ^ b;
}

/**
 * Multiply two elements in GF(256)
 * 
 * @param a First element
 * @param b Second element
 * @return a × b in GF(256)
 * 
 * Uses peasant multiplication with reduction by 0x11B
 */
uint8_t gf256_mul(uint8_t a, uint8_t b);

/**
 * Divide two elements in GF(256)
 * 
 * @param a Numerator
 * @param b Denominator (must not be 0)
 * @return a ÷ b in GF(256)
 * 
 * Division is multiplication by multiplicative inverse: 
 *   a / b = a × b⁻¹
 */
uint8_t gf256_div(uint8_t a, uint8_t b);

/**
 * Calculate multiplicative inverse in GF(256)
 * 
 * @param a Element to invert (must not be 0)
 * @return a⁻¹ such that a × a⁻¹ = 1
 * 
 * Uses Extended Euclidean Algorithm
 */
uint8_t gf256_inv(uint8_t a);

/**
 * Raise an element to a power in GF(256)
 * 
 * @param base The base element
 * @param exp  The exponent
 * @return base^exp in GF(256)
 * 
 * Uses exponentiation by squaring
 */
uint8_t gf256_pow(uint8_t base, uint8_t exp);

/**
 * Initialize lookup tables for fast multiplication
 * (Optional optimization - can be called once at startup)
 * 
 * @return 0 on success, -1 on error
 */
int gf256_init_tables(void);

#ifdef __cplusplus
}
#endif

#endif /* SSS_FIELD_H */

