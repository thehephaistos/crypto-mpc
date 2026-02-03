#include "sss/mpc.h"
#include "sss/secret_sharing.h"
#include "sss/field.h"
#include "utils/secure_memory.h"
#include "utils/error.h"
#include "utils/random.h"
#include <string.h>
#include <stdlib.h>

/* ========================================================================
 * Context Management Functions
 * ======================================================================== */

int mpc_init_context(mpc_context_t *ctx, uint8_t num_parties, 
                     uint8_t threshold, size_t value_size) {
    // Validate parameters
    if (ctx == NULL) {
        return -1;
    }
    
    if (num_parties < 2 || num_parties > 255) {
        return -1;
    }
    
    if (threshold < 2 || threshold > num_parties) {
        return -1;
    }
    
    if (value_size == 0 || value_size > SSS_MAX_SECRET_SIZE) {
        return -1;
    }
    
    // Initialize context
    ctx->num_parties = num_parties;
    ctx->threshold = threshold;
    ctx->value_size = value_size;
    
    // Generate a random computation ID
    ctx->computation_id = sss_random_byte();
    
    return 0;
}

void mpc_cleanup_context(mpc_context_t *ctx) {
    if (ctx == NULL) {
        return;
    }
    
    // Wipe sensitive data using secure_wipe()
    secure_wipe(ctx, sizeof(mpc_context_t));
}

/* ========================================================================
 * Share Distribution Functions
 * ======================================================================== */

int mpc_create_shares(const mpc_context_t *ctx, const uint8_t *secret,
                      mpc_share_t *shares) {
    // Validate inputs
    if (ctx == NULL || secret == NULL || shares == NULL) {
        return -1;
    }
    
    // Create temporary array for SSS shares
    sss_share_t *sss_shares = (sss_share_t *)malloc(ctx->num_parties * sizeof(sss_share_t));
    if (sss_shares == NULL) {
        return -1;
    }
    
    // Use SSS to create shares
    int result = sss_create_shares(secret, ctx->value_size,
                                   ctx->threshold, ctx->num_parties,
                                   sss_shares);
    if (result != SSS_OK) {
        secure_wipe(sss_shares, ctx->num_parties * sizeof(sss_share_t));
        free(sss_shares);
        return -1;
    }
    
    // Wrap SSS shares into MPC shares
    for (uint8_t i = 0; i < ctx->num_parties; i++) {
        shares[i].share = sss_shares[i];
        shares[i].party_id = i + 1;  // Party IDs are 1-based
        shares[i].computation_id = ctx->computation_id;
    }
    
    // Clean up
    secure_wipe(sss_shares, ctx->num_parties * sizeof(sss_share_t));
    free(sss_shares);
    return 0;
}

int mpc_reconstruct(const mpc_context_t *ctx, const mpc_share_t *shares,
                    uint8_t num_shares, uint8_t *reconstructed) {
    // Validate inputs
    if (ctx == NULL || shares == NULL || reconstructed == NULL) {
        return -1;
    }
    
    if (num_shares < ctx->threshold) {
        return -1;
    }
    
    // Extract SSS shares from MPC shares
    sss_share_t *sss_shares = (sss_share_t *)malloc(num_shares * sizeof(sss_share_t));
    if (sss_shares == NULL) {
        return -1;
    }
    
    for (uint8_t i = 0; i < num_shares; i++) {
        // Validate each share
        if (mpc_validate_share(ctx, &shares[i]) != 0) {
            secure_wipe(sss_shares, num_shares * sizeof(sss_share_t));
            free(sss_shares);
            return -1;
        }
        sss_shares[i] = shares[i].share;
    }
    
    // Use SSS to reconstruct
    size_t reconstructed_len = ctx->value_size;
    int result = sss_combine_shares(sss_shares, num_shares,
                                    reconstructed, &reconstructed_len);
    
    // Clean up
    secure_wipe(sss_shares, num_shares * sizeof(sss_share_t));
    free(sss_shares);
    
    return (result == SSS_OK) ? 0 : -1;
}

/* ========================================================================
 * Utility Functions
 * ======================================================================== */

void mpc_wipe_share(mpc_share_t *share) {
    if (share == NULL) {
        return;
    }
    
    // Wipe the underlying SSS share
    sss_wipe_share(&share->share);
    
    // Wipe the MPC metadata
    secure_wipe(share, sizeof(mpc_share_t));
}

int mpc_validate_share(const mpc_context_t *ctx, const mpc_share_t *share) {
    if (ctx == NULL || share == NULL) {
        return -1;
    }
    
    // Check party_id is in valid range
    if (share->party_id < 1 || share->party_id > ctx->num_parties) {
        return -1;
    }
    
    // Check computation_id matches
    if (share->computation_id != ctx->computation_id) {
        return -1;
    }
    
    // Check data length matches expected size
    if (share->share.data_len != ctx->value_size) {
        return -1;
    }
    
    return 0;
}

/* ========================================================================
 * Secure Arithmetic Operations
 * ======================================================================== */

int mpc_secure_add(const mpc_context_t *ctx, const mpc_share_t *shares_x,
                   const mpc_share_t *shares_y, mpc_share_t *shares_sum,
                   uint8_t num_shares) {
    // Validate inputs
    if (ctx == NULL || shares_x == NULL || shares_y == NULL || 
        shares_sum == NULL) {
        return -1;
    }
    
    if (num_shares == 0) {
        return -1;
    }
    
    // Process each share
    for (uint8_t i = 0; i < num_shares; i++) {
        // Validate both input shares
        if (mpc_validate_share(ctx, &shares_x[i]) != 0) {
            return -1;
        }
        if (mpc_validate_share(ctx, &shares_y[i]) != 0) {
            return -1;
        }
        
        // Check that party IDs match
        if (shares_x[i].party_id != shares_y[i].party_id) {
            return -1;  // Shares must be from same party
        }
        
        // Check that data lengths match
        if (shares_x[i].share.data_len != shares_y[i].share.data_len) {
            return -1;
        }
        
        // Initialize output share
        shares_sum[i].party_id = shares_x[i].party_id;
        shares_sum[i].computation_id = ctx->computation_id;
        shares_sum[i].share.index = shares_x[i].share.index;
        shares_sum[i].share.data_len = shares_x[i].share.data_len;
        shares_sum[i].share.threshold = shares_x[i].share.threshold;
        
        // Perform addition in GF(256) for each byte
        for (size_t j = 0; j < shares_x[i].share.data_len; j++) {
            shares_sum[i].share.data[j] = gf256_add(
                shares_x[i].share.data[j],
                shares_y[i].share.data[j]
            );
        }
    }
    
    return 0;
}

int mpc_secure_sub(const mpc_context_t *ctx, const mpc_share_t *shares_x,
                   const mpc_share_t *shares_y, mpc_share_t *shares_diff,
                   uint8_t num_shares) {
    // Validate inputs
    if (ctx == NULL || shares_x == NULL || shares_y == NULL || 
        shares_diff == NULL) {
        return -1;
    }
    
    if (num_shares == 0) {
        return -1;
    }
    
    // Process each share
    for (uint8_t i = 0; i < num_shares; i++) {
        // Validate both input shares
        if (mpc_validate_share(ctx, &shares_x[i]) != 0) {
            return -1;
        }
        if (mpc_validate_share(ctx, &shares_y[i]) != 0) {
            return -1;
        }
        
        // Check that party IDs match
        if (shares_x[i].party_id != shares_y[i].party_id) {
            return -1;
        }
        
        // Check that data lengths match
        if (shares_x[i].share.data_len != shares_y[i].share.data_len) {
            return -1;
        }
        
        // Initialize output share
        shares_diff[i].party_id = shares_x[i].party_id;
        shares_diff[i].computation_id = ctx->computation_id;
        shares_diff[i].share.index = shares_x[i].share.index;
        shares_diff[i].share.data_len = shares_x[i].share.data_len;
        shares_diff[i].share.threshold = shares_x[i].share.threshold;
        
        // Perform subtraction in GF(256) for each byte
        for (size_t j = 0; j < shares_x[i].share.data_len; j++) {
            shares_diff[i].share.data[j] = gf256_sub(
                shares_x[i].share.data[j],
                shares_y[i].share.data[j]
            );
        }
    }
    
    return 0;
}

int mpc_secure_mul_const(const mpc_context_t *ctx, const mpc_share_t *shares_x,
                         uint8_t constant, mpc_share_t *shares_prod,
                         uint8_t num_shares) {
    // Validate inputs
    if (ctx == NULL || shares_x == NULL || shares_prod == NULL) {
        return -1;
    }
    
    if (num_shares == 0) {
        return -1;
    }
    
    // Process each share
    for (uint8_t i = 0; i < num_shares; i++) {
        // Validate input share
        if (mpc_validate_share(ctx, &shares_x[i]) != 0) {
            return -1;
        }
        
        // Initialize output share
        shares_prod[i].party_id = shares_x[i].party_id;
        shares_prod[i].computation_id = ctx->computation_id;
        shares_prod[i].share.index = shares_x[i].share.index;
        shares_prod[i].share.data_len = shares_x[i].share.data_len;
        shares_prod[i].share.threshold = shares_x[i].share.threshold;
        
        // Perform multiplication in GF(256) for each byte
        for (size_t j = 0; j < shares_x[i].share.data_len; j++) {
            shares_prod[i].share.data[j] = gf256_mul(
                shares_x[i].share.data[j],
                constant
            );
        }
    }
    
    return 0;
}

int mpc_secure_mul(const mpc_context_t *ctx, const mpc_share_t *shares_x,
                   const mpc_share_t *shares_y, mpc_share_t *shares_prod,
                   uint8_t num_shares) {
    // ====================================================================
    // Step 1: Validate all inputs
    // ====================================================================
    
    if (ctx == NULL || shares_x == NULL || shares_y == NULL || 
        shares_prod == NULL) {
        return -1;
    }
    
    if (num_shares == 0 || num_shares < ctx->threshold) {
        return -1;  // Need at least threshold shares
    }
    
    // Validate all input shares
    for (uint8_t i = 0; i < num_shares; i++) {
        if (mpc_validate_share(ctx, &shares_x[i]) != 0) {
            return -1;
        }
        if (mpc_validate_share(ctx, &shares_y[i]) != 0) {
            return -1;
        }
        
        // Ensure party IDs match
        if (shares_x[i].party_id != shares_y[i].party_id) {
            return -1;
        }
        
        // Ensure data lengths match
        if (shares_x[i].share.data_len != shares_y[i].share.data_len) {
            return -1;
        }
    }
    
    size_t data_len = shares_x[0].share.data_len;
    
    // ====================================================================
    // Step 2: Local multiplication - each party multiplies their shares
    // ====================================================================
    
    // Allocate secure memory for intermediate shares
    mpc_share_t *intermediate = secure_malloc(num_shares * sizeof(mpc_share_t));
    if (intermediate == NULL) {
        return -1;
    }
    
    // Lock the intermediate memory
    secure_lock(intermediate, num_shares * sizeof(mpc_share_t));
    
    // Each party multiplies their shares element-wise in GF(256)
    for (uint8_t i = 0; i < num_shares; i++) {
        // Copy metadata
        intermediate[i].party_id = shares_x[i].party_id;
        intermediate[i].computation_id = ctx->computation_id;
        intermediate[i].share.index = shares_x[i].share.index;
        intermediate[i].share.data_len = data_len;
        intermediate[i].share.threshold = ctx->threshold;
        
        // Multiply each byte in GF(256)
        // This is the LOCAL computation each party does
        for (size_t j = 0; j < data_len; j++) {
            intermediate[i].share.data[j] = gf256_mul(
                shares_x[i].share.data[j],
                shares_y[i].share.data[j]
            );
        }
    }
    
    // Note: At this point, intermediate shares represent points on a
    // degree-2 polynomial (because we multiplied two degree-1 polynomials).
    // We need degree reduction!
    
    // ====================================================================
    // Step 3: Reconstruct the product
    // ====================================================================
    
    // Allocate secure memory for the product
    uint8_t *product = secure_malloc(data_len);
    if (product == NULL) {
        secure_unlock(intermediate, num_shares * sizeof(mpc_share_t));
        secure_free(intermediate, num_shares * sizeof(mpc_share_t));
        return -1;
    }
    secure_lock(product, data_len);
    
    // Reconstruct using the intermediate shares
    // Note: In production MPC, this step would use Beaver triples
    // to avoid reconstructing the intermediate value.
    // Our simplified version reconstructs here for educational purposes.
    int recon_result = mpc_reconstruct(ctx, intermediate, num_shares, product);
    
    if (recon_result != 0) {
        // Reconstruction failed - cleanup and return error
        secure_unlock(product, data_len);
        secure_free(product, data_len);
        secure_unlock(intermediate, num_shares * sizeof(mpc_share_t));
        secure_free(intermediate, num_shares * sizeof(mpc_share_t));
        return -1;
    }
    
    // ====================================================================
    // Step 4: Reshare the product (Degree Reduction)
    // ====================================================================
    
    // Create new shares of the product as a degree-1 polynomial
    // This is the "degree reduction" step!
    int reshare_result = mpc_create_shares(ctx, product, shares_prod);
    
    // ====================================================================
    // Step 5: Secure cleanup
    // ====================================================================
    
    // Wipe and free intermediate shares
    for (uint8_t i = 0; i < num_shares; i++) {
        secure_wipe(&intermediate[i], sizeof(mpc_share_t));
    }
    secure_unlock(intermediate, num_shares * sizeof(mpc_share_t));
    secure_free(intermediate, num_shares * sizeof(mpc_share_t));
    
    // Wipe and free product
    secure_unlock(product, data_len);
    secure_free(product, data_len);
    
    return reshare_result;
}

/* ========================================================================
 * High-Level MPC Functions
 * ======================================================================== */

int mpc_secure_sum(const mpc_context_t *ctx, 
                   const mpc_share_t **share_sets,
                   uint8_t num_values,
                   uint8_t num_shares,
                   mpc_share_t *shares_sum) {
    // Validate inputs
    if (ctx == NULL || share_sets == NULL || shares_sum == NULL) {
        return -1;
    }
    
    if (num_values == 0 || num_shares == 0) {
        return -1;
    }
    
    // Start with first value's shares
    for (uint8_t i = 0; i < num_shares; i++) {
        // Deep copy first share set
        shares_sum[i] = share_sets[0][i];
    }
    
    // Add each subsequent value
    for (uint8_t val = 1; val < num_values; val++) {
        mpc_share_t temp[num_shares];
        
        // Add this value to running sum
        if (mpc_secure_add(ctx, shares_sum, share_sets[val], 
                          temp, num_shares) != 0) {
            return -1;
        }
        
        // Copy result back to shares_sum
        for (uint8_t i = 0; i < num_shares; i++) {
            shares_sum[i] = temp[i];
        }
    }
    
    return 0;
}

int mpc_secure_average(const mpc_context_t *ctx,
                       const mpc_share_t **share_sets,
                       uint8_t num_values,
                       uint8_t num_shares,
                       uint8_t *average) {
    // Validate inputs
    if (average == NULL) {
        return -1;
    }
    
    // Allocate shares for sum
    mpc_share_t *shares_sum = secure_malloc(num_shares * sizeof(mpc_share_t));
    if (shares_sum == NULL) {
        return -1;
    }
    secure_lock(shares_sum, num_shares * sizeof(mpc_share_t));
    
    // Compute sum
    if (mpc_secure_sum(ctx, share_sets, num_values, num_shares, 
                       shares_sum) != 0) {
        secure_unlock(shares_sum, num_shares * sizeof(mpc_share_t));
        secure_free(shares_sum, num_shares * sizeof(mpc_share_t));
        return -1;
    }
    
    // Reconstruct sum
    uint8_t sum;
    if (mpc_reconstruct(ctx, shares_sum, num_shares, &sum) != 0) {
        secure_unlock(shares_sum, num_shares * sizeof(mpc_share_t));
        secure_free(shares_sum, num_shares * sizeof(mpc_share_t));
        return -1;
    }
    
    // Divide by count (in plaintext)
    // Note: In GF(256), division is different. For simplicity, we use integer division.
    // For production, implement secure division protocol.
    *average = sum / num_values;
    
    // Cleanup
    secure_unlock(shares_sum, num_shares * sizeof(mpc_share_t));
    secure_free(shares_sum, num_shares * sizeof(mpc_share_t));
    
    return 0;
}

int mpc_secure_max(const mpc_context_t *ctx,
                   const mpc_share_t **share_sets,
                   uint8_t num_values,
                   uint8_t num_shares,
                   uint8_t *maximum,
                   uint8_t *max_index) {
    // Validate inputs
    if (ctx == NULL || share_sets == NULL || maximum == NULL) {
        return -1;
    }
    
    if (num_values == 0 || num_shares == 0) {
        return -1;
    }
    
    // Allocate secure memory for reconstructed values
    uint8_t *values = secure_malloc(num_values);
    if (values == NULL) {
        return -1;
    }
    secure_lock(values, num_values);
    
    // Reconstruct all values
    // Note: In production, use secure comparison circuits to avoid reconstruction
    for (uint8_t i = 0; i < num_values; i++) {
        if (mpc_reconstruct(ctx, share_sets[i], num_shares, &values[i]) != 0) {
            secure_unlock(values, num_values);
            secure_free(values, num_values);
            return -1;
        }
    }
    
    // Find maximum
    uint8_t max_val = values[0];
    uint8_t max_idx = 0;
    
    for (uint8_t i = 1; i < num_values; i++) {
        if (values[i] > max_val) {
            max_val = values[i];
            max_idx = i;
        }
    }
    
    *maximum = max_val;
    if (max_index != NULL) {
        *max_index = max_idx;
    }
    
    // Cleanup
    secure_unlock(values, num_values);
    secure_free(values, num_values);
    
    return 0;
}

int mpc_secure_greater(const mpc_context_t *ctx,
                       const mpc_share_t *shares_x,
                       const mpc_share_t *shares_y,
                       uint8_t num_shares,
                       uint8_t *result) {
    // Validate inputs
    if (ctx == NULL || shares_x == NULL || shares_y == NULL || result == NULL) {
        return -1;
    }
    
    // Reconstruct both values
    // Note: Production systems use secure comparison circuits
    uint8_t x, y;
    
    if (mpc_reconstruct(ctx, shares_x, num_shares, &x) != 0) {
        return -1;
    }
    
    if (mpc_reconstruct(ctx, shares_y, num_shares, &y) != 0) {
        return -1;
    }
    
    // Compare
    *result = (x > y) ? 1 : 0;
    
    return 0;
}
