#!/usr/bin/env python3
"""
Create configuration files for AIRS components
"""

import json
from pathlib import Path

def create_environment_config(root):
    """Create global environment configuration"""
    config = {
        "airs": {
            "version": "1.0.0",
            "root_path": "D:/airs",
            "components": {
                "ssot": {
                    "enabled": True,
                    "data_path": "ssot/data",
                    "api_path": "ssot/api",
                    "tools_path": "ssot/tools"
                },
                "simulation": {
                    "enabled": True,
                    "bin_path": "sim/bin",
                    "cli_path": "sim/cli"
                }
            },
            "workspace": {
                "input_path": "workspace/input",
                "output_path": "workspace/output",
                "temp_path": "workspace/temp"
            },
            "logging": {
                "level": "INFO",
                "format": "json",
                "output": "logs/"
            }
        }
    }

    config_path = root / "environment.json"
    config_path.write_text(json.dumps(config, indent=2), encoding='utf-8')
    print(f"[OK] Created: environment.json")

def create_ssot_config(root):
    """Create SSOT configuration"""
    config = {
        "ssot": {
            "version": "1.0.0",
            "data": {
                "database_path": "data/ssot_parallel.db",
                "graph_path": "data/rmap_graph.gpickle",
                "index_path": "data/ssot_index",
                "index_db_path": "data/ssot_index.db"
            },
            "api": {
                "host": "localhost",
                "port": 8080,
                "max_results": 500,
                "default_sensitivity": 1,
                "default_mode": "full_text"
            },
            "search_backends": {
                "fts5": {
                    "enabled": True,
                    "priority": 1,
                    "description": "SQLite FTS5 with BM25 ranking"
                },
                "whoosh": {
                    "enabled": True,
                    "priority": 2,
                    "description": "Whoosh Python search engine"
                },
                "cpp_avx2": {
                    "enabled": True,
                    "priority": 3,
                    "description": "C++ AVX2 optimized indexer/searcher"
                }
            },
            "audit": {
                "enabled": True,
                "log_path": "audit/query_audit.jsonl",
                "format": "jsonl"
            },
            "security": {
                "path_jailing": True,
                "doc_id_validation": True,
                "max_doc_size_mb": 50,
                "max_workspace_size_mb": 500
            }
        }
    }

    config_path = root / "ssot" / "config.json"
    config_path.write_text(json.dumps(config, indent=2), encoding='utf-8')
    print(f"[OK] Created: ssot/config.json")

    # Create README
    readme = """# SSOT - Single Source of Truth

Knowledge management system with multi-backend search capabilities.

## Components

- **Data**: Authoritative database (2.3GB)
  - `ssot_parallel.db` - Main document database with FTS5
  - `rmap_graph.gpickle` - Relational graph
  - `ssot_index/` - Whoosh search index

- **API**: JSON-based search API
  - Strict JSON schemas in `api/schemas/`
  - Search modes: keyword, full_text
  - Sensitivity levels: 0-3

- **Tools**: Utilities for search and inspection
  - `search.py` - Command-line search tool
  - `inspector.py` - Data inspector

- **C++ Index**: High-performance AVX2 indexer/searcher
  - Build with CMake in `cpp_index/`

## Configuration

Edit `config.json` to customize:
- Search backends and priorities
- API settings
- Security parameters
- Audit logging

## Usage (Agent-Oriented)

### Search via JSON API:
```json
{
  "query": "your search terms",
  "mode": "full_text",
  "sensitivity": 1,
  "limit": 20
}
```

### Response Format:
```json
{
  "status": "success",
  "query": "your search terms",
  "total_results": 42,
  "execution_time_ms": 12.5,
  "results": [
    {
      "doc_id": "abc123...",
      "score": 3.14,
      "snippet": "...matching text...",
      "metadata": {...}
    }
  ]
}
```

See `api/schemas/` for complete JSON schemas.
"""

    readme_path = root / "ssot" / "README.md"
    readme_path.write_text(readme, encoding='utf-8')
    print(f"[OK] Created: ssot/README.md")

def create_simulation_config(root):
    """Create Simulation configuration"""
    config = {
        "simulation": {
            "version": "1.0.0",
            "cli": {
                "executable": "bin/dase_cli.exe",
                "json_strict": True,
                "timeout_seconds": 300,
                "validate_schemas": True
            },
            "engines": {
                "default": "igsoa_complex",
                "available": [
                    "igsoa_complex",
                    "igsoa_complex_2d",
                    "igsoa_complex_3d",
                    "igsoa_gw",
                    "satp_higgs_1d",
                    "satp_higgs_2d",
                    "satp_higgs_3d",
                    "phase4b"
                ]
            },
            "cache": {
                "enabled": True,
                "path": "cache/",
                "max_size_mb": 1024,
                "categories": [
                    "fractional_kernels",
                    "fftw_wisdom",
                    "echo_templates",
                    "state_profiles",
                    "surrogates"
                ]
            },
            "output": {
                "format": "json",
                "precision": 6,
                "compression": False,
                "validate_schema": True
            },
            "performance": {
                "enable_avx2": True,
                "enable_openmp": True,
                "num_threads": 0
            }
        }
    }

    config_path = root / "sim" / "config.json"
    config_path.write_text(json.dumps(config, indent=2), encoding='utf-8')
    print(f"[OK] Created: sim/config.json")

    # Create README
    readme = """# Simulation - IGSOA-SIM Framework

High-performance CLI-based simulation framework for physics research.

## Engines

- **DASE**: Distributed Analog Solver Engine (AVX2-optimized)
- **IGSOA**: Complex field simulations (1D/2D/3D + gravitational waves)
- **SATP**: Spatially Asynchronous Temporal Processing
- **SID-SSP**: Semantic Inference DAG + Stability Processor

## CLI Interface

All commands use strict JSON:

### List Engines:
```json
{"command": "list_engines"}
```

### Create Engine:
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

### Evolve System:
```json
{
  "command": "evolve",
  "params": {
    "timesteps": 100,
    "dt": 0.01
  }
}
```

### Get State:
```json
{
  "command": "get_state",
  "params": {
    "format": "json"
  }
}
```

## Response Format

```json
{
  "command": "create_engine",
  "status": "success",
  "execution_time_ms": 45.2,
  "result": {
    "engine_type": "igsoa_complex",
    "num_nodes": 4096,
    "initialized": true
  }
}
```

## Configuration

Edit `config.json` to customize:
- Default engine
- Cache settings
- Output format
- Performance options

See `cli/schemas/` for complete JSON schemas.

## Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_CLI=ON
cmake --build build --config Release
```

Executable: `bin/dase_cli.exe`
"""

    readme_path = root / "sim" / "README.md"
    readme_path.write_text(readme, encoding='utf-8')
    print(f"[OK] Created: sim/README.md")

def create_integration_readme(root):
    """Create integration documentation"""
    readme = """# Integration - Cross-Component Workflows

Enables communication between SSOT and Simulation components via JSON.

## Design Principles

1. **Strict JSON**: All communication uses validated JSON schemas
2. **Stateless**: No shared state between components
3. **File-based**: Data exchange via workspace files
4. **Agent-friendly**: Designed for autonomous agent operation

## Example Workflow

### 1. Query SSOT for Knowledge
```json
{
  "query": "gravitational wave detection",
  "mode": "full_text",
  "sensitivity": 2,
  "limit": 50
}
```
Output: `workspace/output/ssot_results/query_001.json`

### 2. Configure Simulation Based on Results
Use SSOT results to create simulation configuration:
```json
{
  "command": "create_engine",
  "params": {
    "engine_type": "igsoa_gw",
    "num_nodes": 8192,
    "R_c": 2.0
  }
}
```

### 3. Run Simulation
```bash
sim/bin/dase_cli.exe < workspace/input/mission_001.json > workspace/output/simulation_results/run_001.json
```

### 4. Analyze Results
Results are in strict JSON format for agent processing.

## Schemas

- `schemas/pipeline.schema.json` - Workflow pipeline definition

## Scripts

- `scripts/query_and_simulate.py` - Full pipeline example
- `scripts/validate_json.py` - Schema validator

## Testing

- `tests/test_ssot_output.py` - Validate SSOT JSON output
- `tests/test_simulation_input.py` - Validate simulation JSON input
- `tests/test_e2e_workflow.py` - End-to-end integration test
"""

    readme_path = root / "integration" / "README.md"
    readme_path.write_text(readme, encoding='utf-8')
    print(f"[OK] Created: integration/README.md")

def create_main_readme(root):
    """Create main project README"""
    readme = """# AIRS - AI Research Suite

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
"""

    readme_path = root / "README.md"
    readme_path.write_text(readme, encoding='utf-8')
    print(f"[OK] Created: README.md")

def main():
    root = Path("D:/airs")

    print("Creating configuration files for AIRS...")
    print("=" * 60)

    create_environment_config(root)
    create_ssot_config(root)
    create_simulation_config(root)
    create_integration_readme(root)
    create_main_readme(root)

    print("=" * 60)
    print("All configuration files created successfully!")
    print()
    print("Files created:")
    print("  - environment.json")
    print("  - ssot/config.json")
    print("  - ssot/README.md")
    print("  - sim/config.json")
    print("  - sim/README.md")
    print("  - integration/README.md")
    print("  - README.md")

if __name__ == "__main__":
    main()
