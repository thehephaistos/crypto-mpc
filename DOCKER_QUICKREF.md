# 🐳 Docker Quick Reference

## ✅ Problem Fixed!
The container no longer freezes. It now runs tests automatically and exits.

---

## 🎯 Three Ways to Use Docker

### 1️⃣ Quick Test (Recommended)
```bash
docker-compose up --build
```
- ✅ Builds project
- ✅ Runs all 4 tests  
- ✅ Shows results
- ✅ Exits automatically
- ⏱️ ~30-60 seconds

### 2️⃣ Interactive Shell (Debugging)
```bash
docker-compose run --rm crypto-lib bash
```
- ✅ Opens bash prompt in container
- ✅ Full control
- ✅ Run tests manually
- ✅ Exit with `exit` command

### 3️⃣ Clean Build (CI/CD Style)
```bash
./scripts/docker-test.sh
```
- ✅ Complete rebuild
- ✅ No cache
- ✅ Auto cleanup
- ⏱️ ~2-3 minutes

---

## 🚀 Common Commands

| Task | Command |
|------|---------|
| Build and test | `docker-compose up --build` |
| Interactive shell | `docker-compose run --rm crypto-lib bash` |
| Clean rebuild | `./scripts/docker-test.sh` |
| Stop container | `docker-compose down` |
| View logs | `docker-compose logs` |
| Cleanup all | `docker system prune -af` |

---

## 📁 Key Files

- **docker-compose.yml** - Main config (runs tests)
- **Dockerfile** - Container definition
- **scripts/docker-build.sh** - Quick build script
- **scripts/docker-test.sh** - Clean rebuild script
- **docker-compose.override.yml.example** - Interactive mode (optional)

---

## 🔧 What Changed?

### Before (❌ Froze)
```yaml
command: /bin/bash  # Started interactive shell, waited forever
```

### After (✅ Works)
```yaml
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
  ./mpc_foundation_test
  "
```

---

## 💡 Tips

**Editing Code?** Just run `docker-compose up --build` after each change.

**Need to debug?** Use `docker-compose run --rm crypto-lib bash` for a shell.

**CI/CD testing?** Use `./scripts/docker-test.sh` for clean builds.

**Container won't stop?** Press `Ctrl+C` or run `docker-compose down`.

---

## 📖 More Details

See [DOCKER_USAGE.md](DOCKER_USAGE.md) for comprehensive documentation.

---

**Your Docker setup is now working correctly! 🎉**
