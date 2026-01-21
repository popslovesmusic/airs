# IGSOA-SIM: Infinite Gauge Scalar Omega Architecture Simulator

High-performance CLI-based simulation framework for DASE, IGSOA, and SATP computational engines.

---

## Quick Start

```bash
# Use shipped binaries (Windows)
Simulation/bin/dase_cli.exe < commands.json

# Or build from source
cmake -B build -DBUILD_CLI=ON
cmake --build build --config Release
# Binaries after build: build/dase_cli/Release/dase_cli.exe (Windows)
#                      build/dase_cli/dase_cli           (Linux/macOS)

# Interactive mode
Simulation/bin/dase_cli.exe
```

📚 **[Documentation Index](docs/INDEX.md)** - Browse all documentation

---

## Project Overview

**IGSOA-SIM** is a high-performance, CLI-based simulation framework featuring:
- **C++ Engines** (13,072 LOC): DASE, IGSOA, SATP with AVX2 optimizations
- **CLI Interface**: Headless JSON-based control for automation and scripting
- **Python Cache Tools** (optional): Kernel caching, profile generation, governance

**Status**: ✅ Production-ready CLI with full engine support

---

## Documentation

All documentation is in the `docs/` folder, organized by category:

### 📚 For Users
- **[Getting Started Guide](docs/getting-started/INSTRUCTIONS.md)** - Setup and first steps
- **[Quick Reference](docs/getting-started/QUICK_REFERENCE.md)** - Common commands
- **[Tutorials](docs/command-center/tutorials/)** - Interactive walkthroughs

### 🔧 For Developers
- **[Project Structure](docs/architecture-design/STRUCTURAL_ANALYSIS.md)** - Full analysis
- **[Architecture Design](docs/architecture-design/)** - System design docs
- **[API Reference](docs/api-reference/)** - JSON CLI commands/responses
- **[Component Docs](docs/components/)** - Backend, CLI, web components

### 📊 For Project Managers
- **[Feature Status](docs/implementation/FEATURE_IMPLEMENTATION_STATUS_UPDATED.md)** - Completion tracking
- **[Remaining Work](docs/roadmap/DO_NOT_FORGET.md)** - TODO list
- **[Project Metrics](docs/reports-analysis/PROJECT_METRICS_SUMMARY.md)** - Stats overview

### 📖 Complete Index
**[Browse all documentation →](docs/INDEX.md)**

---

## Key Features

### Computational Engines
- **DASE**: Distributed Analog Solver Engine with AVX2 SIMD
- **IGSOA**: 2D/3D complex field simulations with gravitational wave support
- **SATP**: Spatially Asynchronous Temporal Processing (1D/2D/3D)
- **Multi-engine support**: Switch between engines at runtime

### CLI Interface
- **JSON-based API**: Headless interface for automation
- **Batch processing**: Process multiple commands from files
- **Real-time metrics**: Performance profiling and telemetry
- **FFT analysis**: Built-in spectral analysis capabilities
- **Cross-platform**: Windows, Linux, macOS support

### Configuration
- `Simulation/context.json` - default context/config for CLI engines
- `config/` (repo root) - shared config sets for workflows (phases, stress, wrapper, harness)

### Python Cache Tools (Optional)
- Fractional kernel caching (2.2x speedup)
- FFTW wisdom store (100-1000x FFT speedup)
- Profile generation (instant startup)
- Echo templates (GW detection)
- Mission graph DAG caching

---

## Quick Stats

```
Languages:         C++ (90%), Python (10%)
Total Code:        ~13,000 lines C++ + utilities
CLI Executable:    Single binary + engine DLLs
Build Time:        ~2 minutes (Release)
Status:            Production-ready CLI
Platform:          Windows/Linux/macOS
```

---

## Technology Stack

**Core**: C++17, AVX2 SIMD, OpenMP
**Libraries**: FFTW3 (FFT), nlohmann/json (JSON parsing)
**Optional**: Python 3.10+ (cache utilities), NumPy, PyTorch
**Build**: CMake 3.15+, MSVC/GCC/Clang

---

## Usage Examples

### Basic Simulation
```json
{
  "command": "create_engine",
  "params": {
    "engine_type": "igsoa_complex",
    "num_nodes": 4096,
    "R_c": 1.0
  }
}
```

### Run Multiple Timesteps
```json
{
  "command": "evolve",
  "params": {
    "timesteps": 100,
    "dt": 0.01
  }
}
```

### Export Results
```json
{
  "command": "get_state",
  "params": {
    "format": "json"
  }
}
```

## Getting Help

1. **CLI Reference**: See `dase_cli/QUICKSTART.txt` for commands
2. **Documentation**: [Browse docs/INDEX.md](docs/INDEX.md)
3. **JSON API**: Check `docs/implementation/HEADLESS_JSON_CLI_ARCHITECTURE.md`

---

## Build Instructions

### Prerequisites
- CMake 3.15+
- C++17 compiler (MSVC 2019+, GCC 9+, Clang 10+)
- FFTW3 library (libfftw3-3.dll/lib in project root)

### Build Steps
```bash
# Configure (CLI-only)
cmake -B build -DBUILD_CLI=ON -DBUILD_ENGINE_DLLS=ON

# Build
cmake --build build --config Release

# Optional: Build with tests
cmake -B build -DBUILD_CLI=ON -DBUILD_TESTS=ON
cmake --build build --config Release
```

### Build Options
- `BUILD_CLI=ON/OFF` - Build CLI executable (default: ON)
- `BUILD_ENGINE_DLLS=ON/OFF` - Build engine DLLs (default: ON)
- `BUILD_TESTS=ON/OFF` - Build test suite (default: OFF)
- `ENABLE_AVX2=ON/OFF` - Enable AVX2 SIMD (default: ON)
- `ENABLE_OPENMP=ON/OFF` - Enable OpenMP (default: ON)

---

## Repository Structure

```
igsoa-sim/
├── src/cpp/              # C++ computational engines
│   ├── analog_universal_node_engine_avx2.cpp  # DASE core
│   ├── igsoa_*.cpp                             # IGSOA engines
│   ├── satp_higgs_*.cpp                        # SATP engines
│   └── igsoa_gw_engine/                        # GW engine
├── dase_cli/             # CLI executable source
│   ├── src/main.cpp      # Entry point
│   ├── src/command_router.cpp
│   └── src/engine_manager.cpp
├── backend/cache/        # Optional Python cache utilities
├── tests/                # C++ test suite
├── docs/                 # Documentation
└── CMakeLists.txt        # Build configuration
```

---

## Python Cache Utilities (Optional)

The `backend/cache/` directory contains optional Python tools for advanced caching:

```bash
# Install Python dependencies (optional)
pip install numpy torch

# Use cache CLI tools
python -m backend.cache.dase_cache_cli --help
python -m backend.cache.dase_governance_cli --help
```

These are **not required** for CLI operation and can be safely ignored for basic usage.

---

## Contributing

See documentation in `docs/` for:
- Architecture: [docs/architecture-design/](docs/architecture-design/)
- CLI Architecture: [docs/implementation/HEADLESS_JSON_CLI_ARCHITECTURE.md](docs/implementation/HEADLESS_JSON_CLI_ARCHITECTURE.md)
- Testing: [docs/testing/](docs/testing/)

---

**📚 Documentation**: [docs/INDEX.md](docs/INDEX.md) | **⚡ Quick Start**: Build and run `./dase_cli/dase_cli.exe`
