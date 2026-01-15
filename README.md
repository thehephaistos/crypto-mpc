# Aegis Crypto Library

**Advanced cryptographic library built on libsodium, featuring:**

- 🔐 Shamir's Secret Sharing (SSS)
- 🛡️ Verifiable Secret Sharing (VSS)
- 🔒 All-or-Nothing Transform (AONT)
- ⚡ High-performance GF(256) arithmetic
- 🎯 Threshold cryptography
- 🔬 Constant-time operations (side-channel resistant)

## Status

🚧 **In Development** - Phase 0: Foundation

## Features (Planned)

### Phase 1: Core Mathematics
- [ ] GF(256) field operations
- [ ] Polynomial evaluation
- [ ] Lagrange interpolation

### Phase 2: Secret Sharing
- [ ] Basic Shamir's Secret Sharing
- [ ] Verifiable Secret Sharing
- [ ] Threshold schemes

### Phase 3: Advanced Features
- [ ] All-or-Nothing Transform
- [ ] Proactive secret sharing
- [ ] Anomaly detection

## Building

```bash
mkdir build
cd build
cmake .. 
make
make test
