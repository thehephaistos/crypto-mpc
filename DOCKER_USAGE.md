# Docker Usage Guide

Complete guide to using Docker with Crypto-MPC for development, testing, and deployment.

## 📦 Quick Start

```bash
# Clone repository
git clone https://github.com/thehephaistosI/crypto-mpc.git
cd crypto-mpc

# Build and run tests (one command!)
docker-compose up --build
```

That's it! Docker handles all dependencies, builds the project, and runs all tests.

---

## 🎯 Use Cases

### 1. Automated Testing (Default)

**What it does:**
- Builds fresh Docker image
- Compiles entire project
- Runs all 7 test suites (42+ tests)
- Exits automatically when done

**Command:**
```bash
docker-compose up --build
```

**Expected Output:**
```
[+] Building 45.2s (11/11) FINISHED
[+] Running 1/1
 ✔ Container sss-crypto-lib Created

Attaching to sss-crypto-lib

========================================
  Building Project in Docker
========================================
-- The C compiler identification is GNU 11.4.0
-- Configuring done
-- Build files written to: /app/build
[100%] Built target mpc_highlevel_test

========================================
  Running Tests
========================================

✓✓✓ ALL TESTS PASSED! ✓✓✓
```

---

### 2. Interactive Development Shell

**What it does:**
- Starts container with interactive bash shell
- Access to all build tools and source code
- Make changes and test immediately

**Command:**
```bash
docker-compose run --rm crypto-lib bash
```

**Inside container:**
```bash
# You're now inside: root@abc123:/app#

# Navigate
ls                    # See project files
cd build              # Go to build directory

# Build
cmake ..
make

# Run specific tests
./simple_test
./mpc_arithmetic_test

# Run examples
./salary_average
./sealed_auction

# Exit container
exit
```

---

### 3. Clean Rebuild (CI/CD Style)

**What it does:**
- Removes old containers and images
- Builds from scratch (no cache)
- Runs all tests
- Reports success/failure

**Command:**
```bash
./scripts/docker-test.sh
```

**Or manually:**
```bash
# Clean everything
docker-compose down
docker system prune -af

# Rebuild without cache
docker-compose build --no-cache

# Run tests
docker-compose up
```

---

### 4. Development with Live Code Changes

**Setup:**
```bash
# Start container with code mounted
docker-compose run --rm crypto-lib bash
```

**Workflow:**
```bash
# Inside container:
cd /app

# Your host code is mounted - changes appear immediately!
# Edit files on your host machine (VSCode, etc.)
# Then rebuild inside container:

cd build
cmake ..
make
./your_test
```

**Note:** The `/app` directory is mounted from your host, so changes you make on your Mac appear instantly in the container!

---

## 🔧 Advanced Usage

### Custom Build Commands

**Build without running:**
```bash
docker-compose build
```

**Force rebuild:**
```bash
docker-compose build --no-cache
```

**Pull latest base image:**
```bash
docker-compose pull
```

---

### Running Specific Tests

**Method 1: Override command**
```bash
docker-compose run --rm crypto-lib bash -c "cd build && ./mpc_arithmetic_test"
```

**Method 2: Interactive shell**
```bash
docker-compose run --rm crypto-lib bash
cd build
./mpc_arithmetic_test
```

---

### Debugging Build Issues

**View build logs:**
```bash
docker-compose build 2>&1 | tee build.log
```

**Check container logs:**
```bash
docker-compose logs
```

**Inspect container:**
```bash
docker-compose run --rm crypto-lib bash
# Poke around, check what's installed
apt list --installed
which gcc
gcc --version
```

---

### Performance: Volume Caching

The `docker-compose.yml` uses a named volume for build caching:

```yaml
volumes:
  - .:/app                    # Source code (live updates)
  - build-cache:/app/build    # Build artifacts (cached)
```

**Benefits:**
- Faster rebuilds (CMake cache preserved)
- Compiled objects reused
- ~10x faster than rebuilding from scratch

**Clear cache if needed:**
```bash
docker volume rm personal_projects_build-cache
```

---

## 🐛 Troubleshooting

### Issue: Container Freezes/Hangs

**Symptom:**
```
Attaching to sss-crypto-lib
[... nothing happens ...]
```

**Solution:**
```bash
# Stop everything
Ctrl+C
docker-compose down

# Remove stuck containers
docker ps -a
docker rm -f $(docker ps -aq)

# Rebuild and retry
docker-compose up --build
```

---

### Issue: Build Fails - "libsodium not found"

**Symptom:**
```
CMake Error: Could NOT find PkgConfig
Could NOT find libsodium
```

**Solution:**

Check `Dockerfile` has correct packages:
```dockerfile
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libsodium-dev \
    pkg-config \     # ← Make sure this is here!
    git
```

Rebuild without cache:
```bash
docker-compose build --no-cache
```

---

### Issue: "Port already in use"

**Not applicable** to this project (we don't expose ports), but if you modify the setup:

```bash
# Find what's using the port
lsof -i :8080

# Stop conflicting container
docker ps
docker stop <container-id>
```

---

### Issue: "No space left on device"

**Docker uses a lot of disk space. Clean up:**

```bash
# Remove unused containers, images, volumes
docker system prune -af --volumes

# Check disk usage
docker system df
```

---

### Issue: Changes Not Reflected

**Problem:** You edited code but container still runs old version.

**Cause:** Using cached image.

**Solution:**
```bash
# Rebuild image
docker-compose build --no-cache

# Or restart container
docker-compose down
docker-compose up --build
```

---

### Issue: Permission Denied (Linux)

**Symptom:**
```
Permission denied: '/app/build'
```

**Solution:**

Linux: Docker runs as root, files created have wrong ownership.

```bash
# Inside container, match host UID
docker-compose run --rm --user $(id -u):$(id -g) crypto-lib bash

# Or fix ownership after
sudo chown -R $USER:$USER build/
```

---

## 🔒 Security Best Practices

### 1. Don't Build Unknown Code

Only build from trusted sources. Dockerfile can execute arbitrary commands.

### 2. Keep Base Image Updated

```bash
# Update to latest Ubuntu packages
docker-compose build --pull --no-cache
```

### 3. Scan for Vulnerabilities

```bash
# Use Docker's built-in scanner
docker scan shamir-secret-sharing:latest
```

### 4. Use Specific Base Image Versions

In production, pin versions:
```dockerfile
FROM ubuntu:22.04@sha256:specific-hash
```

---

## 📊 Performance Tips

### 1. Use BuildKit

Enable Docker BuildKit for faster builds:

```bash
# In ~/.bashrc or ~/.zshrc
export DOCKER_BUILDKIT=1
export COMPOSE_DOCKER_CLI_BUILD=1

# Then rebuild
docker-compose build
```

### 2. Multi-Stage Builds

For smaller images (future optimization):

```dockerfile
# Stage 1: Build
FROM ubuntu:22.04 AS builder
# ... build everything ...

# Stage 2: Runtime (smaller!)
FROM ubuntu:22.04 AS runtime
COPY --from=builder /app/build /app/build
```

### 3. Layer Caching

Order Dockerfile commands from least to most frequently changing:

```dockerfile
# Good (dependencies rarely change)
RUN apt-get update && apt-get install -y ...
COPY CMakeLists.txt .
COPY src/ src/

# Bad (invalidates cache on every code change)
COPY . .
RUN apt-get update && apt-get install -y ...
```

---

## 🚀 CI/CD Integration

### GitHub Actions

```yaml
name: Docker Build & Test

on: [push, pull_request]

jobs:
  docker-test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Build Docker image
        run: docker-compose build
      
      - name: Run tests
        run: docker-compose up --abort-on-container-exit
      
      - name: Clean up
        run: docker-compose down
```

### GitLab CI

```yaml
docker-test:
  image: docker:latest
  services:
    - docker:dind
  script:
    - docker-compose build
    - docker-compose up --abort-on-container-exit
```

---

## 📝 Docker Compose Reference

### Our Configuration

```yaml
services:
  crypto-lib:
    build:
      context: .
      dockerfile: Dockerfile
    image: shamir-secret-sharing:latest
    container_name: sss-crypto-lib
    volumes:
      - .:/app                    # Source code (live)
    working_dir: /app
    command: >
      bash -c "
      cd /app &&
      rm -rf build &&
      mkdir -p build &&
      cd build &&
      cmake .. &&
      make &&
      ./simple_test &&
      ./comprehensive_test &&
      ./secure_memory_test &&
      ./mpc_foundation_test &&
      ./mpc_arithmetic_test &&
      ./mpc_multiplication_test &&
      ./mpc_highlevel_test
      "
```

**Explanation:**
- `build: context: .` - Build from current directory
- `volumes: - .:/app` - Mount source code
- `command:` - Run on container start
- `bash -c "..."` - Execute test sequence

---

## 🎓 Learning Docker

New to Docker? Here are key concepts:

### Images vs Containers

- **Image:** Blueprint (like a class in OOP)
- **Container:** Running instance (like an object)

```bash
docker images              # List images (blueprints)
docker ps                  # List running containers
docker ps -a               # List all containers
```

### Dockerfile vs docker-compose.yml

- **Dockerfile:** How to build an image
- **docker-compose.yml:** How to run containers

### Volumes

Mount host directories into containers:

```yaml
volumes:
  - .:/app                 # . = host path, /app = container path
```

Changes on host → Visible in container immediately!

---

## 🆘 Getting Help

**Docker not working?**

1. Check Docker is running:
   ```bash
   docker --version
   docker ps
   ```

2. Check docker-compose version:
   ```bash
   docker-compose --version
   ```

3. View detailed logs:
   ```bash
   docker-compose up --build 2>&1 | tee docker.log
   ```

4. Ask for help:
   - Open GitHub issue
   - Reach out on LinkedIn: [Oguz Kirmizi](https://linkedin.com/in/oguz-kirmizi)

---

## ✅ Summary: Common Commands

```bash
# Build and run tests (most common)
docker-compose up --build

# Interactive shell
docker-compose run --rm crypto-lib bash

# Clean rebuild
docker-compose down
docker-compose build --no-cache
docker-compose up

# Stop everything
docker-compose down

# Clean up disk space
docker system prune -af

# View logs
docker-compose logs

# Remove volumes
docker volume rm personal_projects_build-cache
```

---

**Docker simplifies development!** No need to install dependencies, worry about system differences, or "works on my machine" issues. 🎉

*Last Updated: February 2026*
