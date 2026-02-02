# Docker Usage Guide

## ✅ Problem Fixed!

The Docker container no longer freezes. The issue was that `command: /bin/bash` in docker-compose.yml was starting an interactive shell that waited for input.

---

## 🚀 Quick Commands

### Run Tests (Default Mode)
```bash
# Method 1: Simple
docker-compose up --build

# Method 2: With cleanup
docker-compose down && docker-compose up --build

# Method 3: Using the script (recommended)
./scripts/docker-build.sh

# Method 4: Clean build from scratch
./scripts/docker-test.sh
```

### Interactive Development Shell
```bash
# Method 1: Using docker-compose run (ignores override file)
docker-compose run --rm crypto-lib bash

# Method 2: Start service with override file
# (rename docker-compose.override.yml.example to docker-compose.override.yml first)
mv docker-compose.override.yml docker-compose.override.yml.disabled
docker-compose up

# To enable interactive mode permanently:
mv docker-compose.override.yml.disabled docker-compose.override.yml
```

---

## 📁 Files Explained

### `docker-compose.yml` (Main file - Runs tests)
- **Default behavior:** Builds project and runs all tests
- **Command:** Rebuilds from source and executes all test binaries
- **Volume:** Mounts current directory so changes are reflected
- **No caching:** Always builds fresh to catch new changes

### `docker-compose.override.yml` (Optional - Interactive mode)
- **Purpose:** Override default behavior for debugging
- **Activates:** When file exists in project root
- **Effect:** Starts interactive bash shell instead of running tests
- **Usage:** For manual testing and exploration

### `scripts/docker-build.sh`
- Quick build and test
- Uses existing cache
- Cleans up after completion
- Shows pass/fail status

### `scripts/docker-test.sh`
- Full clean rebuild with `--no-cache`
- Removes old containers
- Best for ensuring clean slate
- CI/CD style testing

---

## 🔄 Workflow Examples

### Development Workflow (Edit → Test)
```bash
# 1. Edit source files on your host machine
vim src/core/field_arithmetic.c

# 2. Run tests in Docker (picks up changes)
docker-compose up --build

# 3. Tests run automatically and container exits
# 4. Repeat!
```

### Debugging Workflow
```bash
# Start interactive shell
docker-compose run --rm crypto-lib bash

# Inside container:
cd build
cmake .. && make
./simple_test
gdb ./comprehensive_test
exit
```

### Clean Test Run (CI/CD style)
```bash
./scripts/docker-test.sh
```

---

## 🎯 Different Modes

### Mode 1: Automatic Testing (Default)
**When to use:** Normal development, CI/CD, quick testing

```bash
docker-compose up --build
```

**What happens:**
1. ✅ Mounts your source code
2. ✅ Deletes old build directory
3. ✅ Runs CMake
4. ✅ Compiles project
5. ✅ Runs all 4 tests
6. ✅ Exits with success/failure code
7. ✅ Container stops automatically

**Output format:**
```
========================================
  Building Project in Docker
========================================
-- Configuring done
-- Build files written to: /app/build
[100%] Built target comprehensive_test

========================================
  Running Tests
========================================

[Test output...]

========================================
  ✓ All Tests Completed!
========================================
```

### Mode 2: Interactive Shell
**When to use:** Debugging, exploration, manual testing

```bash
docker-compose run --rm crypto-lib bash
```

**What happens:**
1. ✅ Starts container
2. ✅ Drops you into bash shell
3. ✅ You have full control
4. ✅ Exit when done (container auto-removes)

**Example session:**
```bash
$ docker-compose run --rm crypto-lib bash
root@abc123:/app# cd build
root@abc123:/app/build# ls
root@abc123:/app/build# cmake .. && make
root@abc123:/app/build# ./simple_test
root@abc123:/app/build# exit
```

### Mode 3: One-Off Commands
**When to use:** Single test, specific command

```bash
# Run just one test
docker-compose run --rm crypto-lib bash -c "cd build && cmake .. && make && ./simple_test"

# Check file structure
docker-compose run --rm crypto-lib ls -la

# View a header file
docker-compose run --rm crypto-lib cat include/sss/field.h
```

---

## 🛠️ Troubleshooting

### Container Still Freezes
```bash
# Stop everything
docker-compose down

# Check for override file
ls docker-compose.override.yml

# If it exists, rename it temporarily
mv docker-compose.override.yml docker-compose.override.yml.disabled

# Try again
docker-compose up --build
```

### Build Fails
```bash
# Clean everything
docker-compose down
docker system prune -f

# Rebuild from scratch
docker-compose build --no-cache
docker-compose up
```

### Tests Fail
```bash
# Run interactively to see what's wrong
docker-compose run --rm crypto-lib bash

# Inside container, run tests manually:
cd build
cmake .. && make
./simple_test          # Run each test individually
./comprehensive_test
```

### Changes Not Reflected
```bash
# The volume mount should work automatically, but if not:
docker-compose down
docker-compose up --build

# Or force full rebuild:
./scripts/docker-test.sh
```

### Permission Issues
```bash
# If files created in container have wrong permissions:
sudo chown -R $USER:$USER .

# Or run container as your user:
docker-compose run --rm -u $(id -u):$(id -g) crypto-lib bash
```

---

## 📊 Exit Codes

| Exit Code | Meaning |
|-----------|---------|
| 0 | ✅ All tests passed |
| 1-255 | ❌ Test failure or build error |

Check exit code:
```bash
docker-compose up --build
echo $?  # Shows exit code
```

---

## 🔍 Viewing Logs

```bash
# View logs from last run
docker-compose logs

# Follow logs in real-time
docker-compose logs -f

# View specific service logs
docker-compose logs crypto-lib
```

---

## 🧹 Cleanup Commands

```bash
# Stop and remove containers
docker-compose down

# Remove containers and volumes
docker-compose down -v

# Remove everything including images
docker-compose down --rmi all -v

# Full Docker cleanup
docker system prune -af
docker volume prune -f
```

---

## ⚡ Performance Tips

### Faster Builds
1. **Use build cache:** `docker-compose build` (default)
2. **Keep dependencies in Dockerfile:** Layers are cached
3. **Don't use --no-cache:** Unless you need clean build

### Faster Tests
1. **Run specific tests:**
   ```bash
   docker-compose run --rm crypto-lib bash -c "cd build && cmake .. && make && ./simple_test"
   ```

2. **Keep container running for multiple tests:**
   ```bash
   docker-compose run --rm crypto-lib bash
   # Then run tests multiple times inside
   ```

---

## 🎓 Understanding Docker Compose Override

Docker Compose uses a **layered approach**:

1. **docker-compose.yml** (base config)
2. **docker-compose.override.yml** (optional overrides)

If both files exist, Docker Compose **merges** them:
- `docker-compose up` → uses both files
- `docker-compose -f docker-compose.yml up` → uses only base file

**Tip:** Rename override file when not needed:
```bash
# Disable override
mv docker-compose.override.yml docker-compose.override.yml.disabled

# Enable override
mv docker-compose.override.yml.disabled docker-compose.override.yml
```

---

## 🎯 Summary

| Command | Use Case | Behavior |
|---------|----------|----------|
| `docker-compose up --build` | Quick test | Build + run tests, auto-exit |
| `./scripts/docker-build.sh` | Standard testing | Build + test + cleanup |
| `./scripts/docker-test.sh` | Clean CI/CD test | Full rebuild, no cache |
| `docker-compose run --rm crypto-lib bash` | Interactive debug | Shell access |
| `docker-compose down` | Cleanup | Stop and remove |

**Default behavior now:** Container runs tests and exits (no freeze!)

**For interactive shell:** Use `docker-compose run --rm crypto-lib bash`
