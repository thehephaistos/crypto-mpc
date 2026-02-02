#include "sss/secret_sharing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ANSI color codes for pretty output */
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"

/* Helper function to print hex data */
void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("%s", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
        if (i < len - 1) printf(" ");
    }
    printf("\n");
}

/* Helper function to print a share */
void print_share(const sss_share_t *share) {
    printf("  Share #%d (threshold=%d, len=%zu): ", 
           share->index, share->threshold, share->data_len);
    for (size_t i = 0; i < share->data_len && i < 16; i++) {
        printf("%02x", share->data[i]);
        if (i < share->data_len - 1) printf(" ");
    }
    if (share->data_len > 16) {
        printf(".. .");
    }
    printf("\n");
}

int main(void) {
    printf("\n");
    printf(COLOR_BLUE "================================================\n" COLOR_RESET);
    printf(COLOR_BLUE "  Shamir's Secret Sharing - Simple Test\n" COLOR_RESET);
    printf(COLOR_BLUE "================================================\n" COLOR_RESET);
    printf("\n");

    /* Initialize the library */
    printf(COLOR_YELLOW "→ Initializing library...\n" COLOR_RESET);
    int result = sss_init();
    if (result != SSS_OK) {
        printf(COLOR_RED "✗ Initialization failed: %s\n" COLOR_RESET, sss_strerror(result));
        return 1;
    }
    printf(COLOR_GREEN "✓ Library initialized\n" COLOR_RESET);
    printf("\n");

    /* Define our secret */
    const char *secret_string = "Hello, Secret Sharing!";
    const uint8_t *secret = (const uint8_t *)secret_string;
    size_t secret_len = strlen(secret_string);

    printf(COLOR_YELLOW "→ Original secret:\n" COLOR_RESET);
    printf("  \"%s\"\n", secret_string);
    print_hex("  Hex:  ", secret, secret_len);
    printf("  Length: %zu bytes\n", secret_len);
    printf("\n");

    /* Split the secret into 5 shares (threshold = 3) */
    uint8_t threshold = 3;
    uint8_t num_shares = 5;
    sss_share_t shares[5];

    printf(COLOR_YELLOW "→ Splitting secret into %d shares (threshold=%d)...\n" COLOR_RESET, 
           num_shares, threshold);
    
    result = sss_create_shares(secret, secret_len, threshold, num_shares, shares);
    if (result != SSS_OK) {
        printf(COLOR_RED "✗ Failed to create shares: %s\n" COLOR_RESET, sss_strerror(result));
        return 1;
    }
    
    printf(COLOR_GREEN "✓ Shares created successfully!\n" COLOR_RESET);
    printf("\n");

    /* Print all shares */
    printf(COLOR_YELLOW "→ Generated shares:\n" COLOR_RESET);
    for (int i = 0; i < num_shares; i++) {
        print_share(&shares[i]);
    }
    printf("\n");

    /* Reconstruct using shares 1, 3, and 5 (indices 0, 2, 4) */
    printf(COLOR_YELLOW "→ Reconstructing secret using shares #1, #3, and #5...\n" COLOR_RESET);
    
    sss_share_t reconstruction_shares[3];
    reconstruction_shares[0] = shares[0];  /* Share #1 */
    reconstruction_shares[1] = shares[2];  /* Share #3 */
    reconstruction_shares[2] = shares[4];  /* Share #5 */

    uint8_t reconstructed[SSS_MAX_SECRET_SIZE];
    size_t reconstructed_len = sizeof(reconstructed);

    result = sss_combine_shares(reconstruction_shares, 3, reconstructed, &reconstructed_len);
    if (result != SSS_OK) {
        printf(COLOR_RED "✗ Reconstruction failed: %s\n" COLOR_RESET, sss_strerror(result));
        return 1;
    }

    printf(COLOR_GREEN "✓ Secret reconstructed!\n" COLOR_RESET);
    printf("\n");

    /* Verify the reconstruction */
    printf(COLOR_YELLOW "→ Reconstructed secret:\n" COLOR_RESET);
    printf("  \"");
    for (size_t i = 0; i < reconstructed_len; i++) {
        printf("%c", reconstructed[i]);
    }
    printf("\"\n");
    print_hex("  Hex: ", reconstructed, reconstructed_len);
    printf("  Length: %zu bytes\n", reconstructed_len);
    printf("\n");

    /* Compare */
    if (reconstructed_len == secret_len && 
        memcmp(secret, reconstructed, secret_len) == 0) {
        printf(COLOR_GREEN "✓✓✓ SUCCESS! ✓✓✓\n" COLOR_RESET);
        printf(COLOR_GREEN "Original and reconstructed secrets match perfectly!\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "✗✗✗ FAILURE! ✗✗✗\n" COLOR_RESET);
        printf(COLOR_RED "Secrets do not match!\n" COLOR_RESET);
        return 1;
    }

    /* Clean up */
    printf("\n");
    printf(COLOR_YELLOW "→ Wiping shares from memory...\n" COLOR_RESET);
    for (int i = 0; i < num_shares; i++) {
        sss_wipe_share(&shares[i]);
    }
    sss_wipe_memory(reconstructed, sizeof(reconstructed));
    printf(COLOR_GREEN "✓ Memory wiped\n" COLOR_RESET);

    printf("\n");
    printf(COLOR_BLUE "================================================\n" COLOR_RESET);
    printf(COLOR_BLUE "  Test Complete!\n" COLOR_RESET);
    printf(COLOR_BLUE "================================================\n" COLOR_RESET);
    printf("\n");

    return 0;
}
