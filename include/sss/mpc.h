#ifndef SSS_MPC_H
#define SSS_MPC_H

#include "sss/secret_sharing.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Multi-Party Computation (MPC) Foundation
 * 
 * This module provides the foundation for secure multi-party computation
 * using Shamir's Secret Sharing. MPC enables multiple parties to jointly
 * compute functions over their private inputs without revealing those
 * inputs to each other.
 * 
 * Key Concepts:
 * - Parties distribute their private values as secret shares
 * - Computation happens on shares (not on original values)
 * - Only the final result is reconstructed
 * - Individual inputs remain private throughout
 * 
 * Example Use Case:
 *   5 employees want to compute their average salary without anyone
 *   revealing their individual salary. Each person splits their salary
 *   into 5 shares, distributes them, and then the average is computed
 *   on the shares themselves.
 * 
 * This module provides:
 * - Context management for MPC sessions
 * - Share creation and reconstruction
 * - Share validation and secure cleanup
 * 
 * Future modules will add:
 * - Secure arithmetic operations (add, subtract, multiply)
 * - Secure comparison operations
 * - Protocol execution and communication
 * ======================================================================== */

/* ========================================================================
 * Data Structures
 * ======================================================================== */

/**
 * MPC Share Structure
 * 
 * Wraps a standard SSS share with additional metadata for MPC protocols.
 * Each party in the computation holds one or more MPC shares.
 */
typedef struct {
    sss_share_t share;      // Underlying Shamir's Secret Sharing share
    uint8_t party_id;       // ID of the party holding this share (1-255)
    uint8_t computation_id; // ID to track which computation this belongs to
} mpc_share_t;

/**
 * MPC Context Structure
 * 
 * Maintains the configuration and state for a multi-party computation.
 * Each computation session should have its own context.
 */
typedef struct {
    uint8_t num_parties;    // Total number of parties participating (2-255)
    uint8_t threshold;      // Minimum parties needed to reconstruct (2 to num_parties)
    uint8_t computation_id; // Unique ID for this computation session
    size_t value_size;      // Size of values being computed (in bytes, max 256)
} mpc_context_t;

/* ========================================================================
 * Context Management
 * ======================================================================== */

/**
 * Initialize an MPC context for a new computation session.
 * 
 * This function sets up the parameters for a multi-party computation.
 * Each computation session should have its own unique context.
 * 
 * @param ctx           Pointer to context to initialize
 * @param num_parties   Total number of participating parties (2-255)
 * @param threshold     Minimum parties needed to reconstruct (2 to num_parties)
 * @param value_size    Size of values being computed (in bytes, max 256)
 * @return 0 on success, -1 on failure
 * 
 * Example: 5 employees computing average salary, need 3 to reconstruct
 *   mpc_context_t ctx;
 *   mpc_init_context(&ctx, 5, 3, 1);  // 5 parties, threshold 3, 1-byte values
 * 
 * The threshold determines the security level:
 * - threshold=2: Any 2 parties can reconstruct
 * - threshold=3: Need at least 3 parties (more secure)
 * - threshold=num_parties: Need all parties (maximum security)
 */
int mpc_init_context(mpc_context_t *ctx, uint8_t num_parties, 
                     uint8_t threshold, size_t value_size);

/**
 * Clean up an MPC context and securely wipe sensitive data.
 * 
 * Should be called when done with a computation session to ensure
 * all sensitive parameters are properly cleared from memory.
 * 
 * @param ctx  Pointer to context to cleanup
 * 
 * Example:
 *   mpc_context_t ctx;
 *   mpc_init_context(&ctx, 5, 3, 1);
 *   // ... perform computation ...
 *   mpc_cleanup_context(&ctx);  // Always cleanup when done
 */
void mpc_cleanup_context(mpc_context_t *ctx);

/* ========================================================================
 * Share Distribution
 * ======================================================================== */

/**
 * Create MPC shares from a secret value.
 * 
 * Splits a secret value into multiple shares using Shamir's Secret Sharing.
 * Each party receives one share. The secret can only be reconstructed
 * when at least 'threshold' shares are combined.
 * 
 * @param ctx       MPC context (must be initialized)
 * @param secret    Secret value to share (size must equal ctx->value_size)
 * @param shares    Output array for MPC shares (size must equal num_parties)
 * @return 0 on success, -1 on failure
 * 
 * Example: Split my salary into shares for 5 parties
 *   mpc_context_t ctx;
 *   mpc_init_context(&ctx, 5, 3, 1);
 *   
 *   uint8_t my_salary = 50;  // $50,000 (simplified to fit in 1 byte)
 *   mpc_share_t shares[5];
 *   mpc_create_shares(&ctx, &my_salary, shares);
 *   
 *   // Now distribute shares[0] to party 1, shares[1] to party 2, etc.
 * 
 * Security Note:
 *   - Individual shares reveal NO information about the secret
 *   - Need at least 'threshold' shares to reconstruct
 *   - Shares should be distributed over secure channels
 */
int mpc_create_shares(const mpc_context_t *ctx, const uint8_t *secret,
                      mpc_share_t *shares);

/**
 * Reconstruct a secret from MPC shares.
 * 
 * Combines shares using Shamir's Secret Sharing interpolation to
 * recover the original secret value. Requires at least 'threshold'
 * valid shares from the same computation.
 * 
 * @param ctx            MPC context (must match the shares)
 * @param shares         Input shares (minimum = threshold)
 * @param num_shares     Number of shares provided
 * @param reconstructed  Output buffer for reconstructed secret (size = value_size)
 * @return 0 on success, -1 on failure
 * 
 * Example: Combine shares to reveal the computed result
 *   mpc_context_t ctx;
 *   mpc_init_context(&ctx, 5, 3, 1);
 *   
 *   // After computation, collect shares from 3 parties
 *   mpc_share_t collected_shares[3];
 *   // ... collect shares from parties 1, 3, and 5 ...
 *   
 *   uint8_t result;
 *   mpc_reconstruct(&ctx, collected_shares, 3, &result);
 *   printf("Result: %d\n", result);
 * 
 * Security Note:
 *   - Only reconstruct when you want to reveal the result
 *   - Before reconstruction, the secret remains hidden
 *   - All shares must be from the same computation_id
 */
int mpc_reconstruct(const mpc_context_t *ctx, const mpc_share_t *shares,
                    uint8_t num_shares, uint8_t *reconstructed);

/* ========================================================================
 * Utility Functions
 * ======================================================================== */

/**
 * Securely wipe an MPC share and clear sensitive data.
 * 
 * Uses secure memory wiping to ensure the share data cannot be
 * recovered from memory. Should be called when a share is no longer needed.
 * 
 * @param share  Pointer to share to wipe
 * 
 * Example:
 *   mpc_share_t share;
 *   // ... use the share ...
 *   mpc_wipe_share(&share);  // Securely erase when done
 * 
 * Security Note:
 *   - Always wipe shares when no longer needed
 *   - Uses secure_wipe() to prevent compiler optimization
 *   - Prevents secrets from lingering in memory
 */
void mpc_wipe_share(mpc_share_t *share);

/**
 * Validate an MPC share.
 * 
 * Checks if a share is consistent with the given MPC context.
 * This helps detect corrupted shares or shares from different computations.
 * 
 * @param ctx    MPC context
 * @param share  Share to validate
 * @return 0 if valid, -1 if invalid
 * 
 * Validation checks:
 * - party_id is in valid range (1 to num_parties)
 * - computation_id matches context
 * - data_len matches value_size
 * 
 * Example:
 *   if (mpc_validate_share(&ctx, &received_share) != 0) {
 *       fprintf(stderr, "Invalid share received!\n");
 *       return -1;
 *   }
 * 
 * Security Note:
 *   - Always validate shares before using them in computation
 *   - Prevents mixing shares from different computations
 *   - Detects corrupted or tampered shares
 */
int mpc_validate_share(const mpc_context_t *ctx, const mpc_share_t *share);

/* ========================================================================
 * Secure Arithmetic Operations
 * ======================================================================== */

/**
 * Securely add two shared secrets.
 * Given shares of X and shares of Y, compute shares of (X + Y).
 * No party learns X or Y, only the final result.
 * 
 * Mathematical basis:
 *   share_i(X) + share_i(Y) = share_i(X + Y)
 * 
 * @param ctx        MPC context
 * @param shares_x   Shares of first value (array of num_shares)
 * @param shares_y   Shares of second value (array of num_shares)
 * @param shares_sum Output shares of sum (array of num_shares)
 * @param num_shares Number of shares (must be same for X and Y)
 * @return 0 on success, -1 on failure
 * 
 * Example:
 *   Alice has secret: 50
 *   Bob has secret: 30
 *   Both create shares, distributed to 5 parties
 *   Each party computes: their_share_of_50 + their_share_of_30
 *   Result: shares of 80 (nobody learned 50 or 30!)
 */
int mpc_secure_add(const mpc_context_t *ctx, const mpc_share_t *shares_x,
                   const mpc_share_t *shares_y, mpc_share_t *shares_sum,
                   uint8_t num_shares);

/**
 * Securely subtract two shared secrets.
 * Given shares of X and shares of Y, compute shares of (X - Y).
 * 
 * Mathematical basis:
 *   share_i(X) - share_i(Y) = share_i(X - Y)
 * 
 * @param ctx        MPC context
 * @param shares_x   Shares of minuend (array of num_shares)
 * @param shares_y   Shares of subtrahend (array of num_shares)
 * @param shares_diff Output shares of difference (array of num_shares)
 * @param num_shares Number of shares
 * @return 0 on success, -1 on failure
 */
int mpc_secure_sub(const mpc_context_t *ctx, const mpc_share_t *shares_x,
                   const mpc_share_t *shares_y, mpc_share_t *shares_diff,
                   uint8_t num_shares);

/**
 * Securely multiply a shared secret by a public constant.
 * Given shares of X and public constant C, compute shares of (X × C).
 * This is efficient since C is public (everyone knows C).
 * 
 * Mathematical basis:
 *   C × share_i(X) = share_i(C × X)
 * 
 * @param ctx         MPC context
 * @param shares_x    Shares of secret value (array of num_shares)
 * @param constant    Public constant (single byte, 0-255)
 * @param shares_prod Output shares of product (array of num_shares)
 * @param num_shares  Number of shares
 * @return 0 on success, -1 on failure
 * 
 * Example:
 *   Alice has secret salary: 50 (thousand dollars)
 *   Compute yearly: 50 × 12 = 600
 *   Everyone knows the constant 12, but nobody learns 50!
 */
int mpc_secure_mul_const(const mpc_context_t *ctx, const mpc_share_t *shares_x,
                         uint8_t constant, mpc_share_t *shares_prod,
                         uint8_t num_shares);

#ifdef __cplusplus
}
#endif

#endif /* SSS_MPC_H */
