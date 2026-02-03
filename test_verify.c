#include <stdio.h>
#include <stdint.h>

// Include field operations
extern uint8_t gf256_mul(uint8_t a, uint8_t b);
extern uint8_t gf256_add(uint8_t a, uint8_t b);

int main() {
    uint8_t a = 10, b = 5, c = 20;
    
    // In GF(256)
    uint8_t prod = gf256_mul(a, b);
    uint8_t result = gf256_add(prod, c);
    
    printf("In GF(256):\n");
    printf("  %d × %d = %d\n", a, b, prod);
    printf("  %d + %d = %d\n", prod, c, result);
    printf("  So: (10 × 5) + 20 = %d\n\n", result);
    
    // In integers (for comparison)
    printf("In integers:\n");
    printf("  10 × 5 = 50\n");
    printf("  50 + 20 = 70\n");
    printf("\nNote: In GF(256), addition is XOR!\n");
    printf("  50 XOR 20 = %d\n", 50 ^ 20);
    
    return 0;
}
