# AIRS - Proposed Directory Structure

**Date:** 2026-01-18
**Purpose:** Establish a clean, maintainable home for the AI Research Suite with strict JSON I/O

---

## **Design Principles**

1. **Separation of Concerns**: SSOT and Simulation are independent but can communicate via JSON
2. **Strict JSON I/O**: All inter-component communication uses strict JSON schemas
3. **Data Immutability**: Source data (SSOT DB) remains read-only
4. **Clear Data Flow**: Inputs → Processing → Outputs with defined boundaries
5. **Version Control Ready**: Build artifacts and cache excluded via .gitignore

---

## **Recommended Directory Tree**

```
D:/airs/                                    # Root directory
│
├── .gitignore                              # Exclude build/, cache/, *.db, *.obj, *.dll
├── README.md                               # Project overview
├── SYSTEM_ANALYSIS.md                      # Current analysis report
├── environment.json                        # Global configuration (NEW)
│
├── ssot/                                   # SSOT Knowledge System (standalone)
│   ├── README.md                           # SSOT component documentation
│   ├── config.json                         # SSOT-specific configuration (NEW)
│   │
│   ├── data/                               # Authoritative data (2.3GB - IMMUTABLE)
│   │   ├── ssot_parallel.db                # Main document database (1.5GB)
│   │   ├── rmap_graph.gpickle              # Relational graph (457MB)
│   │   ├── ssot_index.db                   # Search index DB (36KB)
│   │   ├── ssot_index/                     # Whoosh index (334MB)
│   │   ├── CHECKSUMS.sha256                # Data integrity verification
│   │   └── manifest.json                   # Data manifest
│   │
│   ├── cpp_index/                          # C++ AVX2 indexer/searcher
│   │   ├── CMakeLists.txt
│   │   ├── README.md
│   │   ├── src/
│   │   │   ├── tokenizer.cpp
│   │   │   ├── indexer.cpp
│   │   │   └── searcher.cpp
│   │   └── build/                          # Build artifacts (gitignored)
│   │
│   ├── tools/                              # SSOT utilities
│   │   ├── search.py                       # Search CLI (JSON output)
│   │   ├── inspector.py                    # Data inspector
│   │   └── rebuild_index.py                # Index rebuilder
│   │
│   ├── api/                                # JSON API layer (NEW)
│   │   ├── search_api.py                   # Search endpoint
│   │   ├── schemas/                        # JSON schemas
│   │   │   ├── search_request.schema.json
│   │   │   ├── search_response.schema.json
│   │   │   └── document.schema.json
│   │   └── validate.py                     # Schema validator
│   │
│   ├── bin/                                # Compiled binaries
│   │   ├── ssot_indexer.exe                # C++ indexer
│   │   ├── ssot_searcher.exe               # C++ searcher
│   │   └── ssot.exe                        # Governance engine (optional)
│   │
│   ├── logs/                               # Runtime logs (gitignored)
│   │   └── search.log
│   │
│   └── audit/                              # Audit trail (gitignored)
│       └── query_audit.jsonl               # Query audit log (JSONL format)
│
├── simulation/                             # Simulation Framework (standalone)
│   ├── README.md                           # Simulation documentation
│   ├── pyproject.toml                      # Python package config
│   ├── CMakeLists.txt                      # C++ build config
│   ├── config.json                         # Simulation-specific config (NEW)
│   │
│   ├── src/                                # Source code
│   │   ├── cpp/                            # C++ engines
│   │   │   ├── analog_universal_node_engine_avx2.cpp
│   │   │   ├── igsoa_*.cpp
│   │   │   ├── satp_*.cpp
│   │   │   ├── igsoa_gw_engine/
│   │   │   ├── sid_ssp/                    # Semantic processor
│   │   │   └── utils/
│   │   │
│   │   └── python/                         # Python utilities (optional)
│   │       └── backend/cache/              # Cache management
│   │
│   ├── cli/                                # Command-line interface
│   │   ├── CMakeLists.txt
│   │   ├── src/
│   │   │   ├── main.cpp
│   │   │   ├── command_router.cpp
│   │   │   └── engine_manager.cpp
│   │   └── schemas/                        # JSON schemas (NEW)
│   │       ├── command.schema.json         # Input command schema
│   │       ├── response.schema.json        # Output response schema
│   │       └── state.schema.json           # State export schema
│   │
│   ├── bin/                                # Compiled executables
│   │   ├── dase_cli.exe                    # Main CLI
│   │   ├── *.dll                           # Engine DLLs
│   │   └── libfftw3-3.dll                  # FFTW library
│   │
│   ├── missions/                           # Simulation configurations
│   │   ├── examples/
│   │   │   ├── basic_igsoa.json
│   │   │   ├── gravitational_wave.json
│   │   │   └── satp_higgs.json
│   │   └── templates/
│   │       ├── engine_config.template.json
│   │       └── mission.template.json
│   │
│   ├── cache/                              # Runtime cache (gitignored)
│   │   ├── fractional_kernels/
│   │   ├── fftw_wisdom/
│   │   ├── echo_templates/
│   │   ├── state_profiles/
│   │   └── surrogates/
│   │
│   ├── build/                              # CMake build output (gitignored)
│   │   ├── Debug/
│   │   └── Release/
│   │
│   ├── tests/                              # Test suite
│   │   ├── unit/
│   │   ├── integration/
│   │   └── benchmarks/
│   │
│   └── docs/                               # Documentation
│       ├── INDEX.md
│       ├── CLI_USAGE_GUIDE.md
│       ├── architecture-design/
│       ├── api-reference/
│       └── getting-started/
│
├── integration/                            # Cross-component integration (NEW)
│   ├── README.md                           # Integration documentation
│   ├── workflows/                          # Workflow definitions
│   │   ├── ssot_to_simulation.json         # SSOT → Simulation pipeline
│   │   └── simulation_to_ssot.json         # Simulation → SSOT pipeline
│   │
│   ├── schemas/                            # Shared JSON schemas
│   │   ├── pipeline.schema.json
│   │   └── data_exchange.schema.json
│   │
│   ├── scripts/                            # Integration scripts
│   │   ├── query_and_simulate.py           # Full pipeline example
│   │   └── validate_json.py                # JSON validator
│   │
│   └── tests/                              # Integration tests
│       ├── test_ssot_output.py
│       ├── test_simulation_input.py
│       └── test_e2e_workflow.py
│
├── workspace/                              # Shared working directory
│   ├── input/                              # Input staging area
│   │   └── .gitkeep
│   ├── output/                             # Output results
│   │   ├── ssot_results/                   # SSOT query results (JSON)
│   │   └── simulation_results/             # Simulation outputs (JSON)
│   └── temp/                               # Temporary files (gitignored)
│
├── scripts/                                # Utility scripts
│   ├── setup_environment.py                # Initial setup
│   ├── validate_installation.py            # Installation validator
│   └── rebuild_all.sh                      # Rebuild all components
│
├── config/                                 # Global configuration (NEW)
│   ├── environment.template.json           # Environment config template
│   ├── paths.json                          # Path definitions
│   └── logging.json                        # Logging configuration
│
└── docs/                                   # Project-wide documentation
    ├── README.md                           # Documentation index
    ├── ARCHITECTURE.md                     # System architecture
    ├── JSON_CONTRACTS.md                   # JSON schema documentation (NEW)
    ├── INTEGRATION_GUIDE.md                # Integration guide (NEW)
    └── TROUBLESHOOTING.md                  # Common issues
```

---

## **Key Changes from Current Structure**

### **1. Renamed & Reorganized:**
```
New folder/SSOT_CONTENT_V1/     →  ssot/data/
New folder/SSOT_Assistant/      →  ssot/tools/ + ssot/api/
Simulation/                     →  simulation/ (no change)
```

### **2. New Directories:**
- `ssot/api/` - JSON API layer with strict schemas
- `simulation/cli/schemas/` - JSON schemas for CLI I/O
- `integration/` - Cross-component workflows
- `workspace/` - Shared working directory
- `config/` - Global configuration

### **3. Removed/Consolidated:**
- `New folder/agent_tools/` → Removed (not needed for standalone operation)
- `New folder/external_tools/` → Removed (external dependency)
- `New folder/SSOT_Assistant/src/` → Reorganized into `ssot/api/` and `ssot/tools/`

---

## **JSON I/O Contracts**

### **SSOT Search API**

**Input Schema** (`ssot/api/schemas/search_request.schema.json`):
```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["query", "mode"],
  "properties": {
    "query": {
      "type": "string",
      "description": "Search query string"
    },
    "mode": {
      "type": "string",
      "enum": ["keyword", "full_text"],
      "description": "Search mode"
    },
    "sensitivity": {
      "type": "integer",
      "minimum": 0,
      "maximum": 3,
      "default": 1,
      "description": "Search sensitivity (0=STRICT, 1=NORMAL, 2=BROAD, 3=DIAGNOSTIC)"
    },
    "limit": {
      "type": "integer",
      "minimum": 1,
      "maximum": 500,
      "default": 20,
      "description": "Maximum results to return"
    },
    "filters": {
      "type": "object",
      "properties": {
        "logic_class": {"type": "string"},
        "format": {"type": "string"}
      }
    }
  }
}
```

**Output Schema** (`ssot/api/schemas/search_response.schema.json`):
```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["status", "query", "results"],
  "properties": {
    "status": {
      "type": "string",
      "enum": ["success", "error"]
    },
    "query": {
      "type": "string"
    },
    "mode": {
      "type": "string"
    },
    "sensitivity": {
      "type": "integer"
    },
    "total_results": {
      "type": "integer"
    },
    "execution_time_ms": {
      "type": "number"
    },
    "results": {
      "type": "array",
      "items": {
        "type": "object",
        "required": ["doc_id", "score"],
        "properties": {
          "doc_id": {"type": "string"},
          "score": {"type": "number"},
          "snippet": {"type": "string"},
          "metadata": {
            "type": "object",
            "properties": {
              "logic_class": {"type": "string"},
              "format": {"type": "string"},
              "original_uri": {"type": "string"}
            }
          }
        }
      }
    },
    "error": {
      "type": "object",
      "properties": {
        "code": {"type": "string"},
        "message": {"type": "string"}
      }
    }
  }
}
```

### **Simulation CLI**

**Input Schema** (`simulation/cli/schemas/command.schema.json`):
```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["command"],
  "properties": {
    "command": {
      "type": "string",
      "enum": [
        "list_engines",
        "describe_engine",
        "create_engine",
        "evolve",
        "get_state",
        "snapshot",
        "analyze"
      ]
    },
    "params": {
      "type": "object",
      "description": "Command-specific parameters"
    }
  }
}
```

**Output Schema** (`simulation/cli/schemas/response.schema.json`):
```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["command", "status"],
  "properties": {
    "command": {"type": "string"},
    "status": {
      "type": "string",
      "enum": ["success", "error"]
    },
    "execution_time_ms": {"type": "number"},
    "result": {
      "type": "object",
      "description": "Command-specific result"
    },
    "error": {
      "type": "object",
      "properties": {
        "code": {"type": "string"},
        "message": {"type": "string"}
      }
    }
  }
}
```

---

## **Configuration Files**

### **Global Environment** (`environment.json`):
```json
{
  "airs": {
    "version": "1.0.0",
    "root_path": "D:/airs",
    "components": {
      "ssot": {
        "enabled": true,
        "data_path": "ssot/data",
        "api_port": 8080
      },
      "simulation": {
        "enabled": true,
        "bin_path": "simulation/bin"
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
```

### **SSOT Config** (`ssot/config.json`):
```json
{
  "ssot": {
    "version": "1.0.0",
    "data": {
      "database_path": "data/ssot_parallel.db",
      "graph_path": "data/rmap_graph.gpickle",
      "index_path": "data/ssot_index"
    },
    "api": {
      "host": "localhost",
      "port": 8080,
      "max_results": 500,
      "default_sensitivity": 1
    },
    "search_backends": {
      "fts5": {"enabled": true, "priority": 1},
      "whoosh": {"enabled": true, "priority": 2},
      "cpp_avx2": {"enabled": true, "priority": 3}
    },
    "audit": {
      "enabled": true,
      "log_path": "audit/query_audit.jsonl"
    }
  }
}
```

### **Simulation Config** (`simulation/config.json`):
```json
{
  "simulation": {
    "version": "1.0.0",
    "cli": {
      "executable": "bin/dase_cli.exe",
      "json_strict": true,
      "timeout_seconds": 300
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
      "enabled": true,
      "path": "cache/",
      "max_size_mb": 1024
    },
    "output": {
      "format": "json",
      "precision": 6,
      "compression": false
    }
  }
}
```

---

## **Data Flow Architecture**

### **Standalone Operation:**

**SSOT:**
```
External Query → ssot/api/search_api.py → ssot/data/*.db
                                        → JSON Response
```

**Simulation:**
```
JSON Command → simulation/bin/dase_cli.exe → Engine
                                           → JSON Response
```

### **Integrated Workflow (Optional):**
```
1. Query SSOT:
   User → ssot/api/search_api.py → workspace/output/ssot_results/query_001.json

2. Configure Simulation:
   workspace/output/ssot_results/query_001.json
   → User/Script → simulation/missions/custom_mission.json

3. Run Simulation:
   simulation/missions/custom_mission.json
   → simulation/bin/dase_cli.exe
   → workspace/output/simulation_results/run_001.json

4. Analyze Results:
   workspace/output/simulation_results/run_001.json
   → User/Analysis Scripts
```

---

## **Migration Steps**

### **Phase 1: Reorganize SSOT**
```bash
# Create new structure
mkdir -p D:/airs/ssot/{data,tools,api,bin,logs,audit}
mkdir -p D:/airs/ssot/api/schemas
mkdir -p D:/airs/ssot/cpp_index

# Move data (2.3GB)
mv "D:/airs/New folder/SSOT_CONTENT_V1/ssot_parallel.db" D:/airs/ssot/data/
mv "D:/airs/New folder/SSOT_CONTENT_V1/rmap_graph.gpickle" D:/airs/ssot/data/
mv "D:/airs/New folder/SSOT_CONTENT_V1/ssot_index" D:/airs/ssot/data/
mv "D:/airs/New folder/SSOT_CONTENT_V1/ssot_index.db" D:/airs/ssot/data/

# Move utilities
mv "D:/airs/New folder/SSOT_CONTENT_V1/search.py" D:/airs/ssot/tools/
mv "D:/airs/New folder/SSOT_CONTENT_V1/inspector.py" D:/airs/ssot/tools/

# Move C++ index
mv "D:/airs/New folder/SSOT_CONTENT_V1/cpp_index/"* D:/airs/ssot/cpp_index/

# Move binaries
mv "D:/airs/New folder/SSOT_CONTENT_V1/bin/ssot.exe" D:/airs/ssot/bin/
```

### **Phase 2: Reorganize Simulation**
```bash
# Create new structure (mostly in place)
mkdir -p D:/airs/simulation/cli/schemas
mkdir -p D:/airs/simulation/missions/{examples,templates}

# Move Simulation directory
mv D:/airs/Simulation/* D:/airs/simulation/

# Organize CLI
mv D:/airs/simulation/dase_cli D:/airs/simulation/cli
```

### **Phase 3: Create Integration Layer**
```bash
# Create integration structure
mkdir -p D:/airs/integration/{workflows,schemas,scripts,tests}
mkdir -p D:/airs/workspace/{input,output/{ssot_results,simulation_results},temp}
mkdir -p D:/airs/config
mkdir -p D:/airs/scripts
```

### **Phase 4: Configuration**
```bash
# Create configuration files
# (Generate environment.json, ssot/config.json, simulation/config.json)

# Create JSON schemas
# (Generate schema files in ssot/api/schemas/ and simulation/cli/schemas/)
```

---

## **.gitignore Recommendations**

```gitignore
# Build artifacts
build/
*.obj
*.o
*.exe
*.dll
*.so
*.dylib
*.pyd

# Cache
cache/
*.cache
__pycache__/
*.pyc

# Logs
logs/
*.log

# Workspace temp
workspace/temp/
workspace/output/*/

# IDE
.vscode/
.idea/
*.swp

# OS
.DS_Store
Thumbs.db

# Large data files (keep versioned via LFS or external)
ssot/data/*.db
ssot/data/*.gpickle
ssot/data/ssot_index/

# Configuration (user-specific)
environment.json
```

---

## **Validation Checklist**

After migration, verify:

- [ ] SSOT search works: `python ssot/tools/search.py "test query"`
- [ ] SSOT returns strict JSON: Validate against schema
- [ ] Simulation CLI works: `simulation/bin/dase_cli.exe < test.json`
- [ ] Simulation returns strict JSON: Validate against schema
- [ ] Configuration files load correctly
- [ ] All paths resolve correctly from `environment.json`
- [ ] Integration tests pass

---

## **Next Steps**

1. **Review this proposal** - Confirm structure meets requirements
2. **Create schema files** - Define strict JSON contracts
3. **Write migration script** - Automate directory reorganization
4. **Update code** - Fix hardcoded paths in SSOT tools
5. **Test components** - Verify standalone operation
6. **Document workflows** - Create integration examples

---

**Proposal Status:** DRAFT - Awaiting Approval
**Estimated Migration Time:** 2-4 hours
**Risk Level:** LOW (data is copied, not moved initially)
