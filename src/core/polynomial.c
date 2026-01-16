#include "sss/polynomial.h"
#include "sss/field.h"
#include <sodium.h>
#include <string.h>

#define POLY_OK 0
#define POLY_ERROR -1

/* ========================================================================
 * Create Random Polynomial
 * ======================================================================== */

int sss_polynomial_create(
    sss_polynomial_t *poly,
    uint8_t secret,
    uint8_t degree
) {
    if (poly == NULL) {
        return POLY_ERROR;
    }
    
    if (degree > SSS_MAX_POLYNOMIAL_DEGREE) {
        return POLY_ERROR;
    }
    
    poly->coefficients[0] = secret;
    poly->degree = degree;
    
    for (uint8_t i = 1; i <= degree; i++) {
        uint8_t coeff;
        do {
            randombytes_buf(&coeff, 1);
        } while (coeff == 0);
        
        poly->coefficients[i] = coeff;
    }
    
    for (uint8_t i = degree + 1; i <= SSS_MAX_POLYNOMIAL_DEGREE; i++) {
        poly->coefficients[i] = 0;
    }
    
    return POLY_OK;
}

/* ========================================================================
 * Evaluate Polynomial (Horner's Method)
 * ======================================================================== */

uint8_t sss_polynomial_evaluate(
    const sss_polynomial_t *poly,
    uint8_t x
) {
    uint8_t result = poly->coefficients[poly->degree];
    
    for (int i = poly->degree - 1; i >= 0; i--) {
        result = gf256_mul(result, x);
        result = gf256_add(result, poly->coefficients[i]);
    }
    
    return result;
}

/* ========================================================================
 * Lagrange Interpolation
 * ======================================================================== */

uint8_t sss_polynomial_interpolate(
    const uint8_t *points_x,
    const uint8_t *points_y,
    uint8_t num_points
) {
    uint8_t secret = 0;
    
    for (uint8_t i = 0; i < num_points; i++) {
        uint8_t basis = 1;
        
        for (uint8_t j = 0; j < num_points; j++) {
            if (i == j) {
                continue;
            }
            
            uint8_t numerator = points_x[j];
            uint8_t denominator = gf256_sub(points_x[j], points_x[i]);
            
            basis = gf256_mul(basis, gf256_div(numerator, denominator));
        }
        
        secret = gf256_add(secret, gf256_mul(points_y[i], basis));
    }
    
    return secret;
}

/* ========================================================================
 * Secure Memory Wiping
 * ======================================================================== */

void sss_polynomial_wipe(sss_polynomial_t *poly) {
    if (poly == NULL) {
        return;
    }
    
    sodium_memzero(poly->coefficients, sizeof(poly->coefficients));
    poly->degree = 0;
}
