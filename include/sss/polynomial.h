#ifndef SSS_POLYNOMIAL_H
#define SSS_POLYNOMIAL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Constants
 * ======================================================================== */

/* Maximum degree of polynomial (threshold - 1) */
#define SSS_MAX_POLYNOMIAL_DEGREE 254

/* ========================================================================
 * Data Structures
 * ======================================================================== */

/**
 * Represents a polynomial in GF(256)
 * 
 * For a threshold k, we need a polynomial of degree k-1:
 *   P(x) = a₀ + a₁x + a₂x² + ... + aₖ₋₁xᵏ⁻¹
 * 
 * where a₀ is the secret
 */
typedef struct {
    uint8_t coefficients[SSS_MAX_POLYNOMIAL_DEGREE + 1];  /* Polynomial coefficients */
    uint8_t degree;                                        /* Degree of polynomial (k-1) */
} sss_polynomial_t;

/* ========================================================================
 * Polynomial Operations
 * ======================================================================== */

/**
 * Create a random polynomial with given secret as constant term
 * 
 * @param poly      Polynomial structure to initialize (output)
 * @param secret    The secret value (becomes a₀)
 * @param degree    Degree of polynomial (threshold - 1)
 * 
 * @return 0 on success, -1 on error
 * 
 * Example:  For threshold=3, degree=2
 *   P(x) = secret + a₁x + a₂x²
 *   where a₁, a₂ are random
 */
int sss_polynomial_create(
    sss_polynomial_t *poly,
    uint8_t secret,
    uint8_t degree
);

/**
 * Evaluate polynomial at a given point using Horner's method
 * 
 * @param poly  The polynomial to evaluate
 * @param x     The point to evaluate at (share index)
 * 
 * @return P(x) - the value of the polynomial at x
 * 
 * Uses Horner's method for efficiency: 
 *   P(x) = a₀ + x(a₁ + x(a₂ + x(... )))
 * 
 * All operations are in GF(256)
 */
uint8_t sss_polynomial_evaluate(
    const sss_polynomial_t *poly,
    uint8_t x
);

/**
 * Reconstruct the constant term (secret) using Lagrange interpolation
 * 
 * @param points_x  Array of x-coordinates (share indices)
 * @param points_y  Array of y-coordinates (share values)
 * @param num_points Number of points (must be >= threshold)
 * 
 * @return The reconstructed secret (a₀)
 * 
 * Uses Lagrange interpolation in GF(256):
 *   P(0) = Σᵢ yᵢ · Lᵢ(0)
 *   where Lᵢ(0) = Πⱼ≠ᵢ (xⱼ / (xⱼ - xᵢ))
 */
uint8_t sss_polynomial_interpolate(
    const uint8_t *points_x,
    const uint8_t *points_y,
    uint8_t num_points
);

/**
 * Securely wipe polynomial from memory
 * 
 * @param poly The polynomial to wipe
 */
void sss_polynomial_wipe(sss_polynomial_t *poly);

#ifdef __cplusplus
}
#endif

#endif /* SSS_POLYNOMIAL_H */

