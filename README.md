# Shamir's Secret Sharing & Cryptographic Library

A production-grade cryptographic library implementing Shamir's Secret Sharing, Multi-Party Computation, and advanced cryptographic primitives.

## Features

- ✅ Shamir's Secret Sharing (SSS)
- ✅ Secure Memory Management
- ✅ Multi-Party Computation (MPC)
- ✅ GF(256) field operations
- ✅ Polynomial evaluation & interpolation
- 🔄 Threshold Signatures (in progress)
- 📋 Homomorphic Encryption (planned)
- 📋 Zero-Knowledge Proofs (planned)

## Quick Start

### Option 1: Docker (Recommended - Isolated & Safe)

**Prerequisites:** Docker and Docker Compose

```bash
# Clone the repository
git clone https://github.com/yourusername/crypto-lib.git
cd crypto-lib

# Build and test in Docker
./scripts/docker-build.sh

# Or use docker-compose directly
docker-compose up --build
```

### Option 2: Native Build

**Prerequisites:**
- CMake 3.10+
- C compiler (GCC or Clang)
- libsodium

**Install dependencies:**

macOS:
```bash
brew install libsodium cmake
```

Ubuntu/Debian:
```bash
sudo apt-get install libsodium-dev cmake build-essential
```

Fedora:
```bash
sudo dnf install libsodium-devel cmake gcc
```

**Build:**
```bash
./scripts/build.sh
```

**Run tests:**
```bash
./scripts/test.sh
```

## Manual Build

```bash
mkdir build
cd build
cmake ..
make

# Run tests
./simple_test
./comprehensive_test
./secure_memory_test
./mpc_foundation_test
```

## Project Structure

```
├── include/          # Public headers
│   ├── sss/          # Core library
│   │   ├── field.h
│   │   ├── polynomial.h
│   │   ├── secret_sharing.h
│   │   └── mpc.h
│   └── utils/        # Utilities
│       ├── random.h
│       ├── error.h
│       └── secure_memory.h
├── src/              # Implementation
│   ├── core/         # Core crypto
│   │   ├── field_arithmetic.c
│   │   ├── polynomial.c
│   │   ├── secret_sharing.c
│   │   └── mpc.c
│   └── utils/        # Utility functions
│       ├── random.c
│       ├── error.c
│       └── secure_memory.c
├── tests/            # Test suite
├── scripts/          # Build scripts
└── Dockerfile        # Docker setup
```

## Usage Example

```c
#include "sss/secret_sharing.h"

// Initialize
sss_init();

// Split secret
const uint8_t secret[] = "My Secret";
sss_share_t shares[5];
sss_create_shares(secret, strlen(secret), 3, 5, shares);

// Reconstruct (any 3 shares)
uint8_t reconstructed[256];
size_t len = sizeof(reconstructed);
sss_combine_shares(shares, 3, reconstructed, &len);

// Clean up
for (int i = 0; i < 5; i++) {
    sss_wipe_share(&shares[i]);
}
```

## Docker Usage

### Build and Run Tests
```bash
docker-compose up --build
```

### Interactive Development
```bash
# Start a shell in the container
docker-compose run --rm crypto-lib bash

# Inside container, you can:
cd build
./simple_test
./comprehensive_test
```

### Cleanup
```bash
docker-compose down
docker volume rm personal_projects_build-cache
```

## Testing

The project includes comprehensive tests:

- **simple_test** - Basic functionality
- **comprehensive_test** - Advanced scenarios
- **secure_memory_test** - Memory security
- **mpc_foundation_test** - Multi-party computation

Run all tests:
```bash
./scripts/test.sh
```

## CI/CD

This project uses GitHub Actions for continuous integration:
- Tests on Ubuntu and macOS
- Docker build verification
- Runs on every push and pull request

## Contributing

Contributions are welcome! Please ensure:
1. All tests pass
2. Code follows project style
3. Add tests for new features
4. Update documentation

## License

MIT License (see [LICENSE](LICENSE))

## Security

This library handles cryptographic operations. For security concerns:
- Report vulnerabilities privately
- Use secure memory management functions
- Follow best practices for key storage
