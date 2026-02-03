# Quick Reference: MPC Secure Multiplication

## Usage Example

```c
#include "sss/mpc.h"

// Setup
mpc_context_t ctx;
mpc_init_context(&ctx, 5, 3, 1);  // 5 parties, threshold 3

// Alice has secret A, Bob has secret B
uint8_t secret_a = 12;
uint8_t secret_b = 8;

// Create shares
mpc_share_t shares_a[5], shares_b[5];
mpc_create_shares(&ctx, &secret_a, shares_a);
mpc_create_shares(&ctx, &secret_b, shares_b);

// Multiply securely (no one learns A or B!)
mpc_share_t shares_product[5];
mpc_secure_mul(&ctx, shares_a, shares_b, shares_product, 5);

// Reconstruct result
uint8_t result;  // Result = 96 (12 × 8)
mpc_reconstruct(&ctx, shares_product, 3, &result);

// Cleanup
for (int i = 0; i < 5; i++) {
    mpc_wipe_share(&shares_a[i]);
    mpc_wipe_share(&shares_b[i]);
    mpc_wipe_share(&shares_product[i]);
}
mpc_cleanup_context(&ctx);
```

## Function Signature

```c
int mpc_secure_mul(
    const mpc_context_t *ctx,      // MPC context
    const mpc_share_t *shares_x,   // Shares of first value
    const mpc_share_t *shares_y,   // Shares of second value
    mpc_share_t *shares_prod,      // Output shares of product
    uint8_t num_shares             // Number of shares
);
```

## Return Values

- `0` - Success
- `-1` - Failure (invalid parameters or insufficient shares)

## Requirements

1. `num_shares >= ctx->threshold`
2. All `shares_x` and `shares_y` must have matching `party_id` ordering
3. Both share sets must have same `computation_id`
4. Both share sets must have same `data_len`

## Real-World Example: Calculate Rectangle Area

```c
// Two parties each have one dimension
uint8_t length = 12;  // Party 1's secret
uint8_t width = 8;    // Party 2's secret

// Setup context
mpc_context_t ctx;
mpc_init_context(&ctx, 5, 3, 1);

// Create and distribute shares
mpc_share_t shares_len[5], shares_wid[5];
mpc_create_shares(&ctx, &length, shares_len);
mpc_create_shares(&ctx, &width, shares_wid);

// Calculate area securely
mpc_share_t shares_area[5];
mpc_secure_mul(&ctx, shares_len, shares_wid, shares_area, 5);

// Reveal result (area = 96)
uint8_t area;
mpc_reconstruct(&ctx, shares_area, 3, &area);
printf("Area: %d square meters\n", area);  // 96

// Nobody learned the individual length or width!
```

## Chain Multiple Operations

```c
// Compute: (A × B) + C
mpc_share_t shares_a[5], shares_b[5], shares_c[5];
mpc_share_t shares_temp[5], shares_result[5];

// Create shares
mpc_create_shares(&ctx, &secret_a, shares_a);
mpc_create_shares(&ctx, &secret_b, shares_b);
mpc_create_shares(&ctx, &secret_c, shares_c);

// First: A × B
mpc_secure_mul(&ctx, shares_a, shares_b, shares_temp, 5);

// Then: (A×B) + C
mpc_secure_add(&ctx, shares_temp, shares_c, shares_result, 5);

// Reconstruct final result
uint8_t final_result;
mpc_reconstruct(&ctx, shares_result, 3, &final_result);
```

## Important Notes

### GF(256) Field Arithmetic
Operations happen in Galois Field GF(256), not regular integers:
- **Multiplication**: Works as expected for most values
- **Addition**: Is XOR operation (not integer addition!)

Example:
```c
// In GF(256):
// 10 × 5 = 50 (multiplication works normally)
// 50 + 20 = 54 (addition is XOR: 50 ^ 20 = 54)
```

### Security Model
- **Current**: Semi-honest (honest-but-curious)
- **Suitable for**: Trusted environments, research, learning
- **Limitation**: Intermediate values revealed during protocol
- **Production upgrade**: Use Beaver multiplication triples

### Performance
- **Complexity**: O(n²) where n = num_shares (polynomial interpolation)
- **Memory**: Allocates secure memory for intermediate values
- **Cleanup**: Automatically wipes all intermediate values

## Testing

Run multiplication tests:
```bash
cd build
./mpc_multiplication_test
```

All tests:
```bash
./scripts/test.sh
```

## Common Errors

### Insufficient Shares
```c
// ERROR: Only 2 shares, but threshold is 3
mpc_secure_mul(&ctx, shares_a, shares_b, shares_prod, 2);
// Returns: -1
```

### Mismatched Party IDs
```c
// ERROR: shares_a and shares_y have different party orderings
// Returns: -1
```

### Wrong Context
```c
// ERROR: shares created with different computation_id
// Returns: -1
```

## Complete Operation List

Your MPC library now supports:

| Operation | Function | Example |
|-----------|----------|---------|
| Addition | `mpc_secure_add()` | X + Y |
| Subtraction | `mpc_secure_sub()` | X - Y |
| Multiply Const | `mpc_secure_mul_const()` | X × C |
| Multiplication | `mpc_secure_mul()` | X × Y |

All operations maintain privacy - no party learns individual secrets!

## Further Reading

See [WEEK5_IMPLEMENTATION_SUMMARY.md](WEEK5_IMPLEMENTATION_SUMMARY.md) for:
- Technical deep dive
- Security considerations
- Production upgrade path
- Complete test results
