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
