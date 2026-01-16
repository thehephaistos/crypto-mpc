#include "sss/field.h"
#include <stddef.h>

/* Irreducible polynomial for GF(256): x^8 + x^4 + x^3 + x + 1 */
#define GF256_POLYNOMIAL 0x11B

/* ========================================================================
 * GF(256) Multiplication
 * ======================================================================== */

uint8_t gf256_mul(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    uint8_t hi_bit_set;
    
    for (int i = 0; i < 8; i++) {
        if (b & 1) {
            result ^= a;
        }
        
        hi_bit_set = (a & 0x80);
        a <<= 1;
        
        if (hi_bit_set) {
            a ^= 0x1B;
        }
        
        b >>= 1;
    }
    
    return result;
}

/* ========================================================================
 * GF(256) Multiplicative Inverse
 * ======================================================================== */

uint8_t gf256_inv(uint8_t a) {
    if (a == 0) {
        return 0;
    }
    
    uint8_t result = 1;
    uint8_t base = a;
    uint8_t exp = 254;
    
    while (exp > 0) {
        if (exp & 1) {
            result = gf256_mul(result, base);
        }
        base = gf256_mul(base, base);
        exp >>= 1;
    }
    
    return result;
}

/* ========================================================================
 * GF(256) Division
 * ======================================================================== */

uint8_t gf256_div(uint8_t a, uint8_t b) {
    if (b == 0) {
        return 0;
    }
    
    return gf256_mul(a, gf256_inv(b));
}

/* ========================================================================
 * GF(256) Exponentiation
 * ======================================================================== */

uint8_t gf256_pow(uint8_t base, uint8_t exp) {
    uint8_t result = 1;
    
    if (exp == 0) {
        return 1;
    }
    if (base == 0) {
        return 0;
    }
    
    while (exp > 0) {
        if (exp & 1) {
            result = gf256_mul(result, base);
        }
        base = gf256_mul(base, base);
        exp >>= 1;
    }
    
    return result;
}

/* ========================================================================
 * Lookup Table Initialization (Optional Optimization)
 * ======================================================================== */

int gf256_init_tables(void) {
    /* Not implemented yet - using direct multiplication */
    return 0;
}

