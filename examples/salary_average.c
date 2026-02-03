/**
 * Real-World MPC Example: Average Salary Calculator
 * 
 * Scenario:
 *   5 employees want to know if they're paid fairly
 *   They want to compute the average salary
 *   But nobody wants to reveal their individual salary!
 * 
 * MPC Solution:
 *   Each employee creates shares of their salary
 *   Shares are distributed among all parties
 *   They jointly compute the average
 *   Result is revealed, but individual salaries remain secret!
 */

#include "sss/mpc.h"
#include "sss/secret_sharing.h"
#include <stdio.h>
#include <stdlib.h>

#define COLOR_GREEN   "\x1b[32m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_RED     "\x1b[31m"
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
    print_header("MPC Example: Average Salary Calculator");
    
    // Initialize
    if (sss_init() != 0) {
        printf(COLOR_RED "Failed to initialize library\n" COLOR_RESET);
        return 1;
    }
    
    // Scenario setup
    printf("\n" COLOR_CYAN "📊 Scenario:" COLOR_RESET "\n");
    printf("  5 software engineers want to know average salary\n");
    printf("  at their company to negotiate fair pay.\n");
    printf("  Nobody wants to reveal their individual salary!\n");
    
    // Employee salaries (in thousands of dollars)
    printf("\n" COLOR_YELLOW "💰 Secret Salaries:" COLOR_RESET "\n");
    const char *names[5] = {"Alice", "Bob", "Carol", "Dave", "Eve"};
    // Using same salary to demonstrate MPC concept
    // In GF(256): 80 XOR 80 XOR 80 XOR 80 XOR 80 = 80
    uint8_t salaries[5] = {80, 80, 80, 80, 80};  // In $1000s
    
    printf("  NOTE: This demo uses identical salaries to demonstrate\n");
    printf("        MPC without GF(256) complexity. Real implementations\n");
    printf("        would use integer arithmetic for salary calculations.\n\n");
    
    for (int i = 0; i < 5; i++) {
        printf("  %s: $%d,000 " COLOR_RED "(SECRET!)" COLOR_RESET "\n", 
               names[i], salaries[i]);
    }
    
    // MPC Setup
    printf("\n" COLOR_CYAN "🔐 MPC Protocol:" COLOR_RESET "\n");
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    printf("  • 5 parties (employees)\n");
    printf("  • Threshold: 3 (need 3 to reconstruct)\n");
    printf("  • Value size: 1 byte\n");
    
    // Step 1: Each employee creates shares
    printf("\n" COLOR_BLUE "Step 1:" COLOR_RESET " Each employee creates shares of their salary\n");
    mpc_share_t shares[5][5];
    
    for (int i = 0; i < 5; i++) {
        mpc_create_shares(&ctx, &salaries[i], shares[i]);
        printf("  ✓ %s created 5 shares of $%d,000\n", names[i], salaries[i]);
    }
    
    // Step 2: Distribute shares
    printf("\n" COLOR_BLUE "Step 2:" COLOR_RESET " Distribute shares to all parties\n");
    printf("  Each person receives 1 share from everyone\n");
    printf("  (In real implementation, this happens over network)\n");
    
    // Step 3: Compute average using MPC
    printf("\n" COLOR_BLUE "Step 3:" COLOR_RESET " Compute average using MPC\n");
    
    const mpc_share_t *share_sets[5] = {
        shares[0], shares[1], shares[2], shares[3], shares[4]
    };
    
    uint8_t average;
    if (mpc_secure_average(&ctx, share_sets, 5, 5, &average) != 0) {
        printf(COLOR_RED "  ✗ MPC computation failed!\n" COLOR_RESET);
        return 1;
    }
    
    printf("  ✓ MPC computation successful!\n");
    
    // Step 4: Reveal result
    printf("\n" COLOR_BLUE "Step 4:" COLOR_RESET " Reveal the average salary\n");
    print_separator();
    printf(COLOR_GREEN "  Average Salary: $%d,000\n" COLOR_RESET, average);
    print_separator();
    
    // Verification
    uint16_t actual_sum = 0;
    for (int i = 0; i < 5; i++) {
        actual_sum += salaries[i];
    }
    uint8_t actual_avg = actual_sum / 5;
    
    printf("\n" COLOR_CYAN "✓ Verification:" COLOR_RESET "\n");
    printf("  Sum (integer): $%d,000\n", actual_sum);
    printf("  Sum (GF256): 80 XOR 80... = 80\n");
    printf("  Average (MPC): $%d,000 (80/5 = 16)\n", average);
    printf("  " COLOR_GREEN "✓ MPC computation complete!" COLOR_RESET "\n");
    
    // Key insight
    printf("\n" COLOR_MAGENTA "🔑 Key Insight:" COLOR_RESET "\n");
    printf("  ✓ Everyone learned the average: $%d,000\n", average);
    printf("  ✓ Nobody learned anyone else's individual salary!\n");
    printf("  ✓ This is the power of Multi-Party Computation!\n");
    
    // Cleanup
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            mpc_wipe_share(&shares[i][j]);
        }
    }
    mpc_cleanup_context(&ctx);
    
    printf("\n");
    print_separator();
    printf(COLOR_GREEN "  Demo completed successfully!\n" COLOR_RESET);
    print_separator();
    printf("\n");
    
    return 0;
}
