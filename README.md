# AIRS - AI Research Suite

Dual-component research platform for AI agents combining knowledge management and physics simulation.

## Components

### 1. SSOT - Knowledge System
- **Location**: `ssot/`
- **Size**: 2.3GB
- **Purpose**: Document search and knowledge retrieval
- **Interface**: JSON API with strict schemas
- **Backends**: SQLite FTS5, Whoosh, C++ AVX2

### 2. Simulation - Physics Framework
- **Location**: `sim/`
- **Size**: 559MB (compiled)
- **Purpose**: High-performance physics simulations
- **Interface**: JSON CLI (command/response)
- **Engines**: DASE, IGSOA, SATP, SID-SSP

## Quick Start

### SSOT Search:
```bash
python ssot/tools/search.py "your query" --mode full_text --limit 20
```

### Simulation:
```bash
echo '{"command":"list_engines"}' | sim/bin/dase_cli.exe
```

## Configuration

- `environment.json` - Global configuration
- `ssot/config.json` - SSOT settings
- `sim/config.json` - Simulation settings

## Agent Usage

Both components use **strict JSON I/O** for agent communication:

1. **SSOT**: Search requests/responses validated against JSON schemas
2. **Simulation**: Commands/responses validated against JSON schemas
3. **Integration**: Workflows defined in JSON pipelines

See `ssot/api/schemas/` and `sim/cli/schemas/` for complete schemas.

## Directory Structure

```
airs/
├── ssot/              # Knowledge system (standalone)
├── sim/               # Simulation framework (standalone)
├── integration/       # Cross-component workflows
├── workspace/         # Shared I/O area
├── config/            # Global configuration
└── docs/              # Documentation
```

## Documentation

- `ssot/README.md` - SSOT component documentation
- `sim/README.md` - Simulation component documentation
- `integration/README.md` - Integration guide
- `SYSTEM_ANALYSIS.md` - Complete system analysis
- `PROPOSED_DIRECTORY_STRUCTURE.md` - Architecture specification

## Build Requirements

### SSOT:
- Python 3.10+
- SQLite3
- Optional: C++ compiler for AVX2 indexer

### Simulation:
- CMake 3.15+
- C++17 compiler (MSVC/GCC/Clang)
- FFTW3 library
- Optional: Python 3.10+ for cache utilities

## Testing

All components include validation:
- JSON schema validation
- Unit tests
- Integration tests

Run validation:
```bash
python scripts/validate_structure.py
```

---

**Status**: Production-ready for agent use
**Version**: 1.0.0
**Last Updated**: 2026-01-18
