/*
 * Example: Simple MPC Foundation Demo
 * 
 * This demonstrates the basic usage of the MPC foundation module.
 * 
 * Scenario: 5 employees want to verify they all have the same salary
 * without revealing their individual salaries to each other.
 */

#include <stdio.h>
#include <stdint.h>
#include <sodium.h>
#include "sss/mpc.h"

int main(void) {
    // Initialize libsodium
    if (sodium_init() < 0) {
        fprintf(stderr, "Failed to initialize libsodium\n");
        return 1;
    }
    
    printf("═══════════════════════════════════════════════\n");
    printf("  MPC Foundation Demo: Secret Sharing\n");
    printf("═══════════════════════════════════════════════\n\n");
    
    // Scenario: 5 parties, need 3 to reconstruct, working with 1-byte values
    mpc_context_t ctx;
    if (mpc_init_context(&ctx, 5, 3, 1) != 0) {
        fprintf(stderr, "Failed to initialize MPC context\n");
        return 1;
    }
    
    printf("Configuration:\n");
    printf("  - Number of parties: %d\n", ctx.num_parties);
    printf("  - Threshold: %d\n", ctx.threshold);
    printf("  - Computation ID: 0x%02X\n\n", ctx.computation_id);
    
    // Alice's secret salary: $75,000 (simplified to 75)
    uint8_t alice_salary = 75;
    printf("Alice's secret: %d\n", alice_salary);
    
    // Create shares
    mpc_share_t shares[5];
    if (mpc_create_shares(&ctx, &alice_salary, shares) != 0) {
        fprintf(stderr, "Failed to create shares\n");
        return 1;
    }
    
    printf("\nShares created:\n");
    for (int i = 0; i < 5; i++) {
        printf("  Party %d: [data=0x", shares[i].party_id);
        for (size_t j = 0; j < shares[i].share.data_len; j++) {
            printf("%02X", shares[i].share.data[j]);
        }
        printf(", computation_id=0x%02X]\n", shares[i].computation_id);
    }
    
    // Validate shares
    printf("\nValidating shares:\n");
    int all_valid = 1;
    for (int i = 0; i < 5; i++) {
        if (mpc_validate_share(&ctx, &shares[i]) == 0) {
            printf("  ✓ Share %d is valid\n", i + 1);
        } else {
            printf("  ✗ Share %d is invalid\n", i + 1);
            all_valid = 0;
        }
    }
    
    if (!all_valid) {
        fprintf(stderr, "\nValidation failed!\n");
        return 1;
    }
    
    // Reconstruct from shares 0, 2, 4 (3 out of 5)
    printf("\nReconstructing from shares 1, 3, and 5:\n");
    mpc_share_t selected_shares[3] = {shares[0], shares[2], shares[4]};
    uint8_t reconstructed = 0;
    
    if (mpc_reconstruct(&ctx, selected_shares, 3, &reconstructed) != 0) {
        fprintf(stderr, "Failed to reconstruct\n");
        return 1;
    }
    
    printf("  Reconstructed value: %d\n", reconstructed);
    
    // Verify reconstruction
    if (reconstructed == alice_salary) {
        printf("  ✓ Reconstruction successful!\n");
    } else {
        printf("  ✗ Reconstruction failed! Got %d, expected %d\n", 
               reconstructed, alice_salary);
    }
    
    // Clean up
    printf("\nCleaning up...\n");
    for (int i = 0; i < 5; i++) {
        mpc_wipe_share(&shares[i]);
    }
    mpc_cleanup_context(&ctx);
    
    printf("✓ All shares and context securely wiped\n\n");
    printf("═══════════════════════════════════════════════\n");
    printf("  Demo completed successfully!\n");
    printf("═══════════════════════════════════════════════\n\n");
    
    return 0;
}
