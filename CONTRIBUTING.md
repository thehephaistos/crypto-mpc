# Contributing to Crypto-MPC

Thank you for your interest in contributing to Crypto-MPC! 🎉

This project implements Multi-Party Computation and cryptographic primitives in C. Contributions from the community help make secure computation more accessible.

## 🤝 Ways to Contribute

### 1. Report Bugs
- Check existing issues first
- Use the bug report template
- Include:
  - Clear description
  - Steps to reproduce
  - Expected vs actual behavior
  - System info (OS, compiler version)
  - Relevant code snippets

### 2. Suggest Features
- Check the roadmap and existing issues
- Open a discussion or issue describing:
  - The problem it solves
  - Proposed solution
  - Use cases
  - Alternative approaches considered

### 3. Improve Documentation
- Fix typos or unclear explanations
- Add examples
- Expand API documentation
- Translate documentation

### 4. Submit Code
See the detailed process below.

---

## 🔧 Development Setup

### Prerequisites
- CMake 3.10+
- C compiler (GCC 7+ or Clang 6+)
- libsodium
- Git
- Docker (optional, for isolated testing)

### Initial Setup

```bash
# Fork the repository on GitHub
# Then clone your fork
git clone https://github.com/YOUR-USERNAME/crypto-mpc.git
cd crypto-mpc

# Add upstream remote
git remote add upstream https://github.com/thehephaistosI/crypto-mpc.git

# Install dependencies (macOS example)
brew install libsodium cmake

# Build
./scripts/build.sh

# Run tests
./scripts/test.sh
```

---

## 📝 Code Contribution Process

### 1. Create a Branch

```bash
# Update your fork
git fetch upstream
git checkout main
git merge upstream/main

# Create feature branch
git checkout -b feature/your-feature-name
# or
git checkout -b bugfix/issue-number-description
```

### 2. Make Your Changes

Follow the code standards below.

### 3. Write/Update Tests

**All new features MUST include tests.**

- Add tests to appropriate test file in `tests/`
- Or create a new test file if needed
- Ensure all tests pass:

```bash
./scripts/test.sh
```

### 4. Commit Your Changes

Use clear, descriptive commit messages:

```bash
# Good commit messages:
git commit -m "Add secure comparison operation to MPC module"
git commit -m "Fix memory leak in polynomial reconstruction"
git commit -m "Update README with Docker usage examples"

# Bad commit messages:
git commit -m "fix bug"
git commit -m "changes"
git commit -m "wip"
```

**Commit message format:**
```
<type>: <short description>

<optional detailed explanation>

<optional issue reference>
```

**Types:**
- `feat:` New feature
- `fix:` Bug fix
- `docs:` Documentation only
- `test:` Adding or updating tests
- `refactor:` Code refactoring
- `perf:` Performance improvement
- `chore:` Maintenance tasks

**Example:**
```
feat: Add secure division operation to MPC

Implements secure division using multiplicative inverse in GF(256).
Includes comprehensive tests and documentation.

Closes #42
```

### 5. Push to Your Fork

```bash
git push origin feature/your-feature-name
```

### 6. Open a Pull Request

1. Go to the main repository on GitHub
2. Click "Pull Requests" → "New Pull Request"
3. Select your fork and branch
4. Fill out the PR template:
   - Clear title
   - Description of changes
   - Related issues
   - Testing performed

---

## 📏 Code Standards

### C Code Style

**Language Standard:**
- C99 (ISO/IEC 9899:1999)
- Use standard library where appropriate

**Formatting:**
- **Indentation:** 4 spaces (NO tabs)
- **Line length:** Max 100 characters
- **Braces:** K&R style (opening brace on same line)

**Example:**
```c
int mpc_secure_add(const mpc_context_t *ctx, const mpc_share_t *shares_x,
                   const mpc_share_t *shares_y, mpc_share_t *shares_sum,
                   uint8_t num_shares) {
    if (ctx == NULL || shares_x == NULL) {
        return -1;
    }
    
    for (uint8_t i = 0; i < num_shares; i++) {
        // Process each share
        shares_sum[i] = compute_sum(shares_x[i], shares_y[i]);
    }
    
    return 0;
}
```

**Naming Conventions:**
- **Functions:** `lowercase_with_underscores`
- **Types:** `lowercase_with_underscores_t` (suffix with `_t`)
- **Constants:** `UPPERCASE_WITH_UNDERSCORES`
- **Macros:** `UPPERCASE_WITH_UNDERSCORES`

**Documentation:**
- Document all public APIs with detailed comments
- Include parameter descriptions, return values, examples
- Use Doxygen-style comments

**Example:**
```c
/**
 * Securely add two shared secrets.
 * 
 * @param ctx        MPC context
 * @param shares_x   Shares of first value
 * @param shares_y   Shares of second value
 * @param shares_sum Output shares of sum
 * @param num_shares Number of shares
 * @return 0 on success, -1 on failure
 */
int mpc_secure_add(const mpc_context_t *ctx, ...);
```

### Memory Management

- Always check `malloc()` return values
- Free all allocated memory
- Use `secure_malloc()` for sensitive data
- Use `secure_wipe()` before freeing sensitive data
- No memory leaks (verify with valgrind)

### Error Handling

- Return 0 for success, -1 for error
- Validate all input parameters
- Clean up on error paths
- Use consistent error codes

---

## 🧪 Testing Requirements

### Test Coverage

**All contributions must include tests:**
- New features → New tests
- Bug fixes → Test that reproduces the bug + verifies fix
- Refactoring → Existing tests must still pass

### Running Tests

```bash
# Run all tests
./scripts/test.sh

# Run specific test
cd build
./mpc_arithmetic_test

# Run with Docker
docker-compose up --build
```

### Writing Tests

- Use clear test names
- Test both success and failure cases
- Test edge cases (boundary values, null pointers, etc.)
- Use ANSI colors for output (follow existing pattern)

**Example Test:**
```c
int test_secure_addition() {
    printf("\n→ Test: Secure Addition\n");
    
    mpc_context_t ctx;
    mpc_init_context(&ctx, 5, 3, 1);
    
    uint8_t a = 50, b = 30;
    // ... create shares ...
    // ... perform addition ...
    // ... verify result ...
    
    int success = (result == 80);
    cleanup();
    return success;
}
```

---

## 🔍 Code Review Process

All pull requests go through code review:

### What Reviewers Look For

1. **Correctness:** Does it work as intended?
2. **Tests:** Are there comprehensive tests?
3. **Code Quality:** Is it readable and maintainable?
4. **Performance:** Are there obvious inefficiencies?
5. **Security:** Are there security concerns?
6. **Documentation:** Is it well-documented?

### Review Timeline

- Initial review: Within 3-5 days
- Follow-up reviews: Within 2 days
- Merge: After approval and passing CI

### Addressing Review Comments

- Respond to all comments
- Make requested changes
- Push updates to your branch (PR updates automatically)
- Mark conversations as resolved when addressed

---

## 🐛 Reporting Issues

### Bug Reports

Use the bug report template and include:

1. **Description:** Clear explanation of the bug
2. **Steps to Reproduce:**
   ```
   1. Initialize context with...
   2. Call function X with...
   3. Observe error...
   ```
3. **Expected Behavior:** What should happen
4. **Actual Behavior:** What actually happens
5. **Environment:**
   - OS and version
   - Compiler and version
   - libsodium version
6. **Additional Context:** Logs, screenshots, etc.

### Feature Requests

Use the feature request template and include:

1. **Problem:** What problem does this solve?
2. **Proposed Solution:** How would you implement it?
3. **Alternatives:** Other approaches considered
4. **Use Cases:** Real-world applications

---

## 📚 Additional Resources

### Understanding the Codebase

- Read the [Architecture Documentation](docs/ARCHITECTURE.md)
- Review existing tests for examples
- Check the [API Documentation](docs/API.md)

### Learning MPC

- [Wikipedia: Secure Multi-Party Computation](https://en.wikipedia.org/wiki/Secure_multi-party_computation)
- Shamir's 1979 paper: "How to Share a Secret"
- [SPDZ Protocol](https://eprint.iacr.org/2011/535.pdf) for advanced MPC

### Cryptography Best Practices

- Use constant-time operations
- Always wipe sensitive data
- Validate all inputs
- Follow the existing patterns

---

## 💬 Getting Help

**Questions about contributing?**

- Open a GitHub Discussion
- Comment on relevant issues
- Reach out on LinkedIn: [Oguz Kirmizi](https://linkedin.com/in/oguz-kirmizi)

**Don't be shy!** No question is too basic. We're here to help.

---

## 🎯 Good First Issues

New to the project? Look for issues labeled:
- `good first issue` - Great for beginners
- `documentation` - Help improve docs
- `help wanted` - Community input needed

---

## 📜 Code of Conduct

### Our Standards

**Be respectful:**
- Friendly and patient
- Welcoming to newcomers
- Respectful of differing viewpoints
- Constructive criticism

**Not acceptable:**
- Harassment or discrimination
- Trolling or insulting comments
- Publishing others' private information
- Unprofessional conduct

### Enforcement

Violations may result in:
- Warning
- Temporary ban
- Permanent ban

Report issues to: GitHub issues or LinkedIn direct message.

---

## 🏆 Recognition

Contributors are recognized in:
- GitHub contributors list
- Release notes
- Project README (for significant contributions)

Thank you for making Crypto-MPC better! 🙏

---

## 📝 Summary Checklist

Before submitting a PR, verify:

- [ ] Code follows style guidelines
- [ ] All tests pass (`./scripts/test.sh`)
- [ ] New tests added for new features
- [ ] Documentation updated
- [ ] Commit messages are clear
- [ ] No compiler warnings
- [ ] Memory leaks checked (if applicable)
- [ ] PR description is complete

---

**Happy Contributing! 🎉**

*Last Updated: February 2026*
