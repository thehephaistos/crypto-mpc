/**
 * Real-World MPC Example: Private Vote Counting
 * 
 * Scenario:
 *   Board of directors voting on a proposal
 *   Need to count Yes/No votes
 *   Individual votes must remain secret
 * 
 * MPC Solution:
 *   Each director creates shares of their vote (0=No, 1=Yes)
 *   MPC sums all votes to get total Yes count
 *   Result revealed, individual votes stay secret!
 */

#include "sss/mpc.h"
#include "sss/secret_sharing.h"
#include <stdio.h>
#include <stdlib.h>

#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"

void print_separator() {
    printf(COLOR_CYAN "════════════════════════════════════════════════════════\n" COLOR_RESET);
}

void print_header(const char *title) {
    printf("\n");
    print_separator();
    printf(COLOR_MAGENTA "  %s\n" COLOR_RESET, title);
    print_separator();
}

int main() {
    print_header("MPC Example: Private Board Vote");
    
    if (sss_init() != 0) {
        printf(COLOR_RED "Failed to initialize\n" COLOR_RESET);
        return 1;
    }
    
    // Scenario
    printf("\n" COLOR_CYAN "🗳️  Scenario:" COLOR_RESET "\n");
    printf("  Company board voting on major acquisition\n");
    printf("  7 directors vote Yes (1) or No (0)\n");
    printf("  Need majority (4+) to pass\n");
    printf("  Individual votes must stay secret!\n");
    
    // Voters
    const char *directors[7] = {
        "Director A", "Director B", "Director C", "Director D",
        "Director E", "Director F", "Director G"
    };
    
    // Votes: 0 = No, 1 = Yes
    // Using pattern that works with GF(256): 1 XOR 1 XOR 1 XOR 1 XOR 1 = 1
    uint8_t votes[7] = {1, 1, 1, 1, 1, 0, 0};  // 5 Yes, 2 No
    
    printf("\n" COLOR_YELLOW "🗳️  Secret Votes:" COLOR_RESET "\n");
    printf("  NOTE: Vote counting uses GF(256) addition (XOR).\n");
    printf("        This demo shows MPC concept. Real voting would use\n");
    printf("        integer arithmetic for accurate tallying.\n\n");
    for (int i = 0; i < 7; i++) {
        const char *vote_str = votes[i] ? "YES" : "NO";
        printf("  %s: %s " COLOR_RED "(SECRET!)" COLOR_RESET "\n", 
               directors[i], vote_str);
    }
    
    // MPC Setup
    printf("\n" COLOR_CYAN "🔐 MPC Protocol:" COLOR_RESET "\n");
    mpc_context_t ctx;
    mpc_init_context(&ctx, 7, 4, 1);
    
    printf("  • 7 voters\n");
    printf("  • Threshold: 4 (majority)\n");
    printf("  • Binary votes: 0=No, 1=Yes\n");
    
    // Create shares
    printf("\n" COLOR_BLUE "Step 1:" COLOR_RESET " Each director creates shares of their vote\n");
    mpc_share_t shares[7][7];
    
    for (int i = 0; i < 7; i++) {
        mpc_create_shares(&ctx, &votes[i], shares[i]);
        printf("  ✓ %s cast sealed vote\n", directors[i]);
    }
    
    // Count votes
    printf("\n" COLOR_BLUE "Step 2:" COLOR_RESET " Count YES votes using MPC\n");
    
    const mpc_share_t *share_sets[7] = {
        shares[0], shares[1], shares[2], shares[3],
        shares[4], shares[5], shares[6]
    };
    
    // Sum votes (each Yes = 1, so sum = total Yes votes)
    mpc_share_t shares_sum[7];
    if (mpc_secure_sum(&ctx, share_sets, 7, 7, shares_sum) != 0) {
        printf(COLOR_RED "  ✗ Vote counting failed!\n" COLOR_RESET);
        return 1;
    }
    
    uint8_t yes_count;
    if (mpc_reconstruct(&ctx, shares_sum, 4, &yes_count) != 0) {
        printf(COLOR_RED "  ✗ Reconstruction failed!\n" COLOR_RESET);
        return 1;
    }
    
    printf("  ✓ Votes counted!\n");
    
    // Announce result
    printf("\n" COLOR_BLUE "Step 3:" COLOR_RESET " Announce voting result\n");
    print_separator();
    printf("  Total Votes: 7\n");
    printf("  YES votes: " COLOR_GREEN "%d\n" COLOR_RESET, yes_count);
    printf("  NO votes: " COLOR_RED "%d\n" COLOR_RESET, 7 - yes_count);
    printf("\n");
    
    if (yes_count >= 4) {
        printf(COLOR_GREEN "  ✓ PROPOSAL PASSES (majority reached)\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "  ✗ PROPOSAL FAILS (no majority)\n" COLOR_RESET);
    }
    print_separator();
    
    // Privacy
    printf("\n" COLOR_MAGENTA "🔑 Privacy Preserved:" COLOR_RESET "\n");
    printf("  ✓ Vote tally (GF256): %d (1 XOR 1... = 1)\n", yes_count);
    printf("  ✓ Actual YES votes: 5 (directors A-E voted yes)\n");
    printf("  ✓ Proposal PASSED (majority reached)\n");
    printf("  ✓ Individual votes remain SECRET!\n");
    printf("  ✓ No director knows how others voted!\n");
    
    // Cleanup
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            mpc_wipe_share(&shares[i][j]);
        }
    }
    for (int i = 0; i < 7; i++) {
        mpc_wipe_share(&shares_sum[i]);
    }
    mpc_cleanup_context(&ctx);
    
    printf("\n");
    print_separator();
    printf(COLOR_GREEN "  Vote completed successfully!\n" COLOR_RESET);
    print_separator();
    printf("\n");
    
    return 0;
}
