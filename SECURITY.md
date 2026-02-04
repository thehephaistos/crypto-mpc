# Security Policy

## 🔒 Security Model

Crypto-MPC implements Multi-Party Computation (MPC) protocols with the following security guarantees:

### Current Security Level: Semi-Honest (Honest-but-Curious)

**Definition:** Parties follow the protocol correctly but may attempt to learn additional information from messages they receive.

**Guarantees:**
- ✅ Privacy: No party learns anything beyond their output
- ✅ Correctness: Output is computed correctly
- ⚠️ Assumption: Parties do not deviate from protocol
- ⚠️ No protection against: Active attacks, malicious adversaries

### Future Roadmap: Malicious Security

We are working on upgrading to malicious security, which provides protection against:
- Active adversaries who deviate from the protocol
- Incorrect share generation
- Tampering with messages
- Aborting the protocol strategically

**Implementation plan:**
1. Add zero-knowledge proofs for share validity
2. Implement verifiable secret sharing (VSS)
3. Add commitment schemes
4. Implement dispute resolution mechanisms

---

## 🐛 Reporting Security Vulnerabilities

**CRITICAL: Do NOT report security vulnerabilities through public GitHub issues!**

### Preferred Method: GitHub Security Advisories

1. Go to the [Security tab](https://github.com/thehephaistosI/crypto-mpc/security)
2. Click "Report a vulnerability"
3. Provide detailed information:
   - Type of vulnerability
   - Affected components
   - Steps to reproduce
   - Potential impact
   - Suggested fixes (if any)

### Alternative Method: Private Contact

If you cannot use GitHub Security Advisories:
- **LinkedIn:** Send a direct message to [Oguz Kirmizi](https://linkedin.com/in/oguz-kirmizi)
- **Subject:** `[SECURITY] Crypto-MPC Vulnerability Report`

### What to Include

1. **Description:** Clear explanation of the vulnerability
2. **Proof of Concept:** Code or steps to demonstrate the issue
3. **Impact Assessment:** What could an attacker achieve?
4. **Affected Versions:** Which versions are vulnerable?
5. **Suggested Fix:** If you have a solution in mind

---

## 📋 Supported Versions

| Version | Supported          | Security Updates |
| ------- | ------------------ | ---------------- |
| 1.0.x   | ✅ Yes            | Full support     |
| < 1.0   | ❌ No             | Upgrade required |

**Recommendation:** Always use the latest stable release.

---

## 🛡️ Security Best Practices

### For Users of This Library

#### 1. Memory Security

```c
// ✅ GOOD: Use secure memory functions
uint8_t *secret = secure_malloc(SECRET_SIZE);
// ... use secret ...
secure_wipe(secret, SECRET_SIZE);
secure_free(secret);

// ❌ BAD: Regular malloc/free leaves data in memory
uint8_t *secret = malloc(SECRET_SIZE);
// ... use secret ...
free(secret);  // Data still in memory!
```

#### 2. Random Number Generation

```c
// ✅ GOOD: Use cryptographically secure RNG
randombytes_buf(buffer, size);  // Uses libsodium

// ❌ BAD: Standard library RNG is NOT cryptographically secure
rand();  // Predictable!
```

#### 3. Input Validation

```c
// ✅ GOOD: Always validate inputs
if (ctx == NULL || shares == NULL || num_shares < threshold) {
    return -1;  // Fail safely
}

// ❌ BAD: Trusting inputs
// Directly using pointers without checks can cause crashes or leaks
```

#### 4. Constant-Time Operations

```c
// ✅ GOOD: Field operations are constant-time
uint8_t result = field_add(a, b);  // Timing-safe

// ❌ BAD: Conditional logic leaks information via timing
if (secret == expected) {
    // Timing attack possible!
}
```

#### 5. Share Distribution

**Never send all shares over a single channel!**

```c
// ✅ GOOD: Different shares to different parties via separate channels
send_to_party_1(shares[0]);  // Party 1 via Channel A
send_to_party_2(shares[1]);  // Party 2 via Channel B
send_to_party_3(shares[2]);  // Party 3 via Channel C

// ❌ BAD: Broadcasting all shares
broadcast(shares);  // Anyone can reconstruct the secret!
```

---

## 🔍 Known Limitations

### 1. Semi-Honest Security Only

**Impact:** Protocol assumes parties follow instructions.

**Mitigation:** Upcoming malicious security upgrade.

**Current Recommendation:** Use in trusted environments only.

### 2. No Network Layer

**Impact:** Library does not handle network communication.

**Mitigation:** Implement secure channels (TLS 1.3+) in your application.

**Example:**
```c
// Your responsibility: Secure channel setup
ssl_context_t *ssl = setup_tls_channel();

// Then use library for MPC
mpc_context_t ctx;
mpc_init_context(&ctx, n, t, party_id);
```

### 3. Side-Channel Resistance

**Timing Attacks:**
- ✅ Field arithmetic is constant-time
- ✅ Polynomial operations are constant-time
- ⚠️ Memory allocation timing may vary

**Cache Attacks:**
- ✅ No secret-dependent memory access in critical paths
- ⚠️ General mitigation (e.g., AES-NI) not implemented

**Power Analysis:**
- ❌ Not designed for hardware side-channel resistance
- Use in software-only environments

### 4. Denial of Service

**Current State:** No protection against DoS attacks.

**Mitigation:** Implement at application layer:
- Rate limiting
- Authentication
- Resource quotas

---

## 🧪 Security Testing

### Our Testing Practices

1. **Unit Tests:** All cryptographic primitives tested
2. **Integration Tests:** End-to-end MPC protocols verified
3. **Memory Safety:** Valgrind checks for leaks
4. **Fuzzing:** Input validation testing (planned)
5. **Code Review:** All changes reviewed for security

### Testing Your Integration

```bash
# Run all security-critical tests
cd build
./secure_memory_test      # Memory wiping verification
./mpc_foundation_test     # Core MPC correctness
./mpc_arithmetic_test     # Arithmetic operations
./comprehensive_test      # End-to-end scenarios

# Check for memory leaks
valgrind --leak-check=full ./your_program
```

---

## 📚 Cryptographic Primitives

### Dependencies

**libsodium:** Used for:
- ✅ Cryptographically secure random number generation (`randombytes_buf`)
- ✅ Constant-time comparisons (`sodium_memcmp`)
- ✅ Secure memory locking (`sodium_mlock`)
- ✅ Secure memory wiping (`sodium_memzero`)

**Custom Implementation:**
- Galois Field GF(256) arithmetic
- Shamir's Secret Sharing
- MPC protocols

**Why not use libsodium for everything?**
- Educational: Demonstrates MPC concepts
- Flexibility: Custom protocol extensions
- Research: Experimental MPC techniques

---

## 🚨 Emergency Response

### If You Discover a Vulnerability

1. **DO NOT** disclose publicly
2. **DO** report via GitHub Security Advisories or LinkedIn
3. **DO** provide details (see "Reporting Security Vulnerabilities")
4. **WAIT** for acknowledgment before public disclosure

### Response Timeline

- **Acknowledgment:** Within 48 hours
- **Initial Assessment:** Within 5 business days
- **Fix Development:** 1-4 weeks depending on severity
- **Public Disclosure:** After patch is released

### Severity Levels

| Level    | Impact                                      | Response Time |
| -------- | ------------------------------------------- | ------------- |
| Critical | Remote code execution, secret exposure      | 24-48 hours   |
| High     | Protocol break, authentication bypass       | 1 week        |
| Medium   | Information disclosure, DoS                 | 2 weeks       |
| Low      | Minor information leak, implementation bug  | 4 weeks       |

---

## 🏆 Security Hall of Fame

We recognize security researchers who responsibly disclose vulnerabilities:

*(No vulnerabilities reported yet - be the first!)*

**Recognition:**
- Listed in SECURITY.md
- Mentioned in release notes
- Credit in CHANGELOG
- GitHub profile link

---

## 📖 Additional Resources

### Academic Papers

1. **Shamir's Secret Sharing:**
   - Shamir, A. (1979). "How to Share a Secret"
   - [Link](https://dl.acm.org/doi/10.1145/359168.359176)

2. **MPC Foundations:**
   - Yao, A. (1982). "Protocols for Secure Computations"
   - Goldreich, O., Micali, S., & Wigderson, A. (1987). "How to Play ANY Mental Game"

3. **Malicious Security:**
   - Damgård, I., et al. (2012). "SPDZ: Practical Protocol for Dishonest Majority MPC"
   - [Link](https://eprint.iacr.org/2011/535.pdf)

### Recommended Reading

- [Wikipedia: Secure Multi-Party Computation](https://en.wikipedia.org/wiki/Secure_multi-party_computation)
- [A Pragmatic Introduction to Secure Multi-Party Computation](https://securecomputation.org/)
- [libsodium Documentation](https://doc.libsodium.org/)

---

## 🔐 Compliance and Standards

### Current Compliance

- ❌ FIPS 140-2/3: Not certified (custom implementations)
- ⚠️ GDPR: Use for privacy-preserving computation (good practice)
- ⚠️ HIPAA: Suitable for privacy-preserving health data analysis
- ❌ Common Criteria: Not evaluated

### Cryptographic Standards

- ✅ Uses libsodium (well-audited library)
- ✅ Based on peer-reviewed academic protocols
- ⚠️ Custom GF(256) implementation (not standardized)
- ⚠️ Semi-honest model only (not suitable for all applications)

---

## ✅ Security Checklist for Developers

Before deploying applications using Crypto-MPC:

- [ ] Understand semi-honest security model limitations
- [ ] Implement secure communication channels (TLS 1.3+)
- [ ] Use secure memory functions for all secrets
- [ ] Validate all inputs from untrusted sources
- [ ] Distribute shares over separate channels
- [ ] Test with comprehensive test suite
- [ ] Run valgrind to check for memory leaks
- [ ] Review code for timing side-channels
- [ ] Implement application-layer authentication
- [ ] Plan for DoS mitigation
- [ ] Have incident response plan
- [ ] Keep library updated to latest version

---

## 📬 Contact

**Security Questions:** GitHub Security Advisories (preferred)

**Other Inquiries:** 
- GitHub Issues (non-security)
- LinkedIn: [Oguz Kirmizi](https://linkedin.com/in/oguz-kirmizi)

---

**Thank you for helping keep Crypto-MPC secure!** 🔒

*Last Updated: February 2026*
