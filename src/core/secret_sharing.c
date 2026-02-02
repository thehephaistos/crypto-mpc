#include "sss/secret_sharing.h"
#include "sss/field.h"
#include "sss/polynomial.h"
#include "utils/random.h"
#include "utils/error.h"
#include <sodium.h>
#include <string.h>
#include <stdlib.h>

/* ========================================================================
 * Library Initialization
 * ======================================================================== */

/**
 * Initialize the secret sharing library
 * 
 * Must be called before using any other functions. 
 * Initializes libsodium for cryptographic operations.
 */
int sss_init(void) {
    /* Initialize libsodium */
    if (sodium_init() < 0) {
        return SSS_ERR_CRYPTO;
    }
    
    return SSS_OK;
}

/* ========================================================================
 * Input Validation
 * ======================================================================== */

/**
 * Validate parameters for share creation
 */
static int validate_share_params(
    const uint8_t *secret,
    size_t secret_len,
    uint8_t threshold,
    uint8_t num_shares,
    const sss_share_t *shares
) {
    /* Check for NULL pointers */
    if (secret == NULL || shares == NULL) {
        return SSS_ERR_INVALID_PARAM;
    }
    
    /* Check secret length */
    if (secret_len == 0 || secret_len > SSS_MAX_SECRET_SIZE) {
        return SSS_ERR_INVALID_PARAM;
    }
    
    /* Check threshold */
    if (threshold < SSS_MIN_THRESHOLD) {
        return SSS_ERR_INVALID_THRESHOLD;
    }
    
    if (threshold > num_shares) {
        return SSS_ERR_INVALID_THRESHOLD;
    }
    
    /* Check number of shares */
    if (num_shares > SSS_MAX_SHARES || num_shares == 0) {
        return SSS_ERR_INVALID_SHARES;
    }
    
    return SSS_OK;
}

/* ========================================================================
 * Create Shares (Split Secret)
 * ======================================================================== */

/**
 * Split a secret into shares using Shamir's Secret Sharing
 * 
 * For each byte of the secret: 
 *   1. Create a random polynomial with the byte as constant term
 *   2. Evaluate the polynomial at different points to generate shares
 *   3. Store the share values
 * 
 * Each share contains: 
 *   - index: The x-coordinate (1 to num_shares)
 *   - threshold: Required number of shares to reconstruct
 *   - data:  The y-coordinates for each byte
 */
int sss_create_shares(
    const uint8_t *secret,
    size_t secret_len,
    uint8_t threshold,
    uint8_t num_shares,
    sss_share_t *shares
) {
    /* Validate parameters */
    int result = validate_share_params(secret, secret_len, threshold, num_shares, shares);
    if (result != SSS_OK) {
        return result;
    }
    
    /* Initialize all shares */
    for (uint8_t i = 0; i < num_shares; i++) {
        shares[i].index = i + 1;  /* Share indices are 1-based (x=0 would reveal secret) */
        shares[i].threshold = threshold;
        shares[i]. data_len = secret_len;
        memset(shares[i].data, 0, SSS_SHARE_DATA_SIZE);
    }
    
    /* Process each byte of the secret */
    for (size_t byte_idx = 0; byte_idx < secret_len; byte_idx++) {
        sss_polynomial_t poly;
        
        /* Create polynomial with this secret byte as constant term */
        /* Degree = threshold - 1 (e.g., threshold=3 needs degree-2 polynomial) */
        if (sss_polynomial_create(&poly, secret[byte_idx], threshold - 1) != 0) {
            return SSS_ERR_CRYPTO;
        }
        
        /* Evaluate polynomial at each share's x-coordinate */
        for (uint8_t share_idx = 0; share_idx < num_shares; share_idx++) {
            uint8_t x = shares[share_idx].index;
            uint8_t y = sss_polynomial_evaluate(&poly, x);
            
            /* Store the y-value in this share */
            shares[share_idx]. data[byte_idx] = y;
        }
        
        /* Wipe polynomial from memory */
        sss_polynomial_wipe(&poly);
    }
    
    return SSS_OK;
}

/* ========================================================================
 * Combine Shares (Reconstruct Secret)
 * ======================================================================== */

/**
 * Reconstruct secret from shares using Lagrange interpolation
 * 
 * For each byte position:
 *   1. Collect the (x, y) points from all shares
 *   2. Use Lagrange interpolation to find P(0) = the secret byte
 *   3. Store in output buffer
 */
int sss_combine_shares(
    const sss_share_t *shares,
    uint8_t num_shares,
    uint8_t *secret,
    size_t *secret_len
) {
    /* Validate inputs */
    if (shares == NULL || secret == NULL || secret_len == NULL) {
        return SSS_ERR_INVALID_PARAM;
    }
    
    if (num_shares == 0) {
        return SSS_ERR_INVALID_SHARES;
    }
    
    /* Check we have enough shares */
    uint8_t threshold = shares[0].threshold;
    if (num_shares < threshold) {
        return SSS_ERR_INVALID_SHARES;
    }
    
    /* Check all shares have same threshold and data length */
    size_t data_len = shares[0].data_len;
    for (uint8_t i = 1; i < num_shares; i++) {
        if (shares[i].threshold != threshold || shares[i].data_len != data_len) {
            return SSS_ERR_INVALID_SHARES;
        }
    }
    
    /* Check for duplicate share indices */
    for (uint8_t i = 0; i < num_shares; i++) {
        for (uint8_t j = i + 1; j < num_shares; j++) {
            if (shares[i]. index == shares[j].index) {
                return SSS_ERR_DUPLICATE_SHARE;
            }
        }
    }
    
    /* Check output buffer size */
    if (*secret_len < data_len) {
        return SSS_ERR_BUFFER_TOO_SMALL;
    }
    
    /* Reconstruct each byte of the secret */
    for (size_t byte_idx = 0; byte_idx < data_len; byte_idx++) {
        /* Collect x and y coordinates for this byte from all shares */
        uint8_t points_x[SSS_MAX_SHARES];
        uint8_t points_y[SSS_MAX_SHARES];
        
        for (uint8_t i = 0; i < num_shares; i++) {
            points_x[i] = shares[i].index;
            points_y[i] = shares[i].data[byte_idx];
        }
        
        /* Use Lagrange interpolation to find P(0) = secret byte */
        secret[byte_idx] = sss_polynomial_interpolate(points_x, points_y, num_shares);
    }
    
    *secret_len = data_len;
    return SSS_OK;
}

/* ========================================================================
 * Share Validation
 * ======================================================================== */

/**
 * Validate a single share's structure and values
 */
int sss_validate_share(const sss_share_t *share) {
    if (share == NULL) {
        return SSS_ERR_INVALID_PARAM;
    }
    
    /* Check index is non-zero (x=0 would reveal secret) */
    if (share->index == 0) {
        return SSS_ERR_INVALID_SHARES;
    }
    
    /* Check threshold */
    if (share->threshold < SSS_MIN_THRESHOLD) {
        return SSS_ERR_INVALID_THRESHOLD;
    }
    
    /* Check data length */
    if (share->data_len == 0 || share->data_len > SSS_SHARE_DATA_SIZE) {
        return SSS_ERR_INVALID_SHARES;
    }
    
    return SSS_OK;
}

/* ========================================================================
 * Error Handling
 * ======================================================================== */

/**
 * Get human-readable error message
 */
const char* sss_strerror(int error_code) {
    return sss_error_string(error_code);
}

/* ========================================================================
 * Secure Memory Operations
 * ======================================================================== */

/**
 * Securely wipe a share from memory
 */
void sss_wipe_share(sss_share_t *share) {
    if (share == NULL) {
        return;
    }
    
    sodium_memzero(share, sizeof(sss_share_t));
}

/**
 * Securely wipe arbitrary data from memory
 */
void sss_wipe_memory(void *data, size_t len) {
    if (data == NULL || len == 0) {
        return;
    }
    
    sodium_memzero(data, len);
}