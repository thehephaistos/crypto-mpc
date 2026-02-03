/**
 * Real-World MPC Example: Sealed-Bid Auction
 * 
 * Scenario:
 *   5 companies are bidding on a government contract
 *   Highest bid wins
 *   But bids should remain secret until winner is determined
 * 
 * MPC Solution:
 *   Each company creates shares of their bid
 *   MPC finds the maximum bid
 *   Winner is announced without revealing losing bids!
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
    print_header("MPC Example: Sealed-Bid Auction");
    
    if (sss_init() != 0) {
        printf(COLOR_RED "Failed to initialize\n" COLOR_RESET);
        return 1;
    }
    
    // Scenario
    printf("\n" COLOR_CYAN "🏛️  Scenario:" COLOR_RESET "\n");
    printf("  Government contract auction\n");
    printf("  5 construction companies submit sealed bids\n");
    printf("  Highest bid wins, but losing bids stay secret\n");
    
    // Bidders
    const char *companies[5] = {
        "BuildCorp",
        "ConstructCo",
        "MegaBuild",
        "QuickBuild",
        "EliteBuild"
    };
    
    uint8_t bids[5] = {85, 92, 78, 95, 88};  // In millions
    
    printf("\n" COLOR_YELLOW "💼 Secret Bids:" COLOR_RESET "\n");
    for (int i = 0; i < 5; i++) {
        printf("  %s: $%dM " COLOR_RED "(SECRET!)" COLOR_RESET "\n", 
               companies[i], bids[i]);
    }
    
    // MPC Setup
    printf("\n" COLOR_CYAN "🔐 MPC Protocol:" COLOR_RESET "\n");
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    printf("  • 5 bidders\n");
    printf("  • Threshold: 3\n");
    printf("  • Sealed-bid protocol\n");
    
    // Create shares
    printf("\n" COLOR_BLUE "Step 1:" COLOR_RESET " Each company creates shares of their bid\n");
    mpc_share_t shares[5][5];
    
    for (int i = 0; i < 5; i++) {
        mpc_create_shares(&ctx, &bids[i], shares[i]);
        printf("  ✓ %s submitted sealed bid\n", companies[i]);
    }
    
    // Find maximum
    printf("\n" COLOR_BLUE "Step 2:" COLOR_RESET " Find highest bid using MPC\n");
    
    const mpc_share_t *share_sets[5] = {
        shares[0], shares[1], shares[2], shares[3], shares[4]
    };
    
    uint8_t winning_bid;
    uint8_t winner_idx;
    
    if (mpc_secure_max(&ctx, share_sets, 5, 5, &winning_bid, &winner_idx) != 0) {
        printf(COLOR_RED "  ✗ Auction failed!\n" COLOR_RESET);
        return 1;
    }
    
    printf("  ✓ Winner determined!\n");
    
    // Announce winner
    printf("\n" COLOR_BLUE "Step 3:" COLOR_RESET " Announce winner\n");
    print_separator();
    printf(COLOR_GREEN "  🏆 WINNER: %s\n" COLOR_RESET, companies[winner_idx]);
    printf(COLOR_GREEN "  💰 Winning Bid: $%dM\n" COLOR_RESET, winning_bid);
    print_separator();
    
    // Privacy preserved
    printf("\n" COLOR_MAGENTA "🔑 Privacy Preserved:" COLOR_RESET "\n");
    printf("  ✓ Winner announced: %s\n", companies[winner_idx]);
    printf("  ✓ Winning bid revealed: $%dM\n", winning_bid);
    printf("  ✓ Losing bids remain SECRET!\n");
    printf("  ✓ Competitors don't learn each other's pricing!\n");
    
    // Cleanup
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            mpc_wipe_share(&shares[i][j]);
        }
    }
    mpc_cleanup_context(&ctx);
    
    printf("\n");
    print_separator();
    printf(COLOR_GREEN "  Auction completed successfully!\n" COLOR_RESET);
    print_separator();
    printf("\n");
    
    return 0;
}
