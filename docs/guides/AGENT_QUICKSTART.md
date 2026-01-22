# AIRS - Agent Quick Start Guide

**AI Research Suite for Autonomous Agents**
**Version:** 1.0.0
**Status:** Production Ready

---

## TL;DR - For Agents

Two standalone components with strict JSON I/O:

1. **SSOT** - Search 1.5GB knowledge base → JSON results
2. **Simulation** - Run physics simulations → JSON results

Both accept JSON on stdin, return JSON on stdout.

---

## Quick Tests

### Test SSOT Search:
```bash
cd D:/airs
echo '{"query":"test","mode":"full_text","limit":5}' | python ssot/api/search_api.py
```

### Test Simulation:
```bash
cd D:/airs
echo '{"command":"list_engines"}' | Simulation/bin/dase_cli.exe
```

---

## SSOT - Knowledge Search

### Input Schema:
```json
{
  "query": "string (required)",
  "mode": "keyword|full_text (required)",
  "sensitivity": 0-3 (optional, default: 1),
  "limit": 1-500 (optional, default: 20),
  "filters": {
    "logic_class": "string (optional)",
    "format": "string (optional)"
  }
}
```

### Output Schema:
```json
{
  "status": "success|error",
  "query": "string",
  "mode": "keyword|full_text",
  "sensitivity": 0-3,
  "total_results": number,
  "execution_time_ms": number,
  "results": [
    {
      "doc_id": "hash",
      "score": number,
      "snippet": "text with <b>matches</b>",
      "metadata": {
        "logic_class": "string",
        "format": "string",
        "original_uri": "string"
      }
    }
  ],
  "error": {
    "code": "string",
    "message": "string"
  }
}
```

### Sensitivity Levels:
- `0` = STRICT (all terms AND) - highest precision
- `1` = NORMAL (core terms AND, last OR) - balanced (default)
- `2` = BROAD (any term OR) - higher recall
- `3` = DIAGNOSTIC (OR for corpus coverage)

### Example Workflow:
```bash
# 1. Create request
cat > request.json << 'EOF'
{
  "query": "gravitational wave detection",
  "mode": "full_text",
  "sensitivity": 2,
  "limit": 50
}
EOF

# 2. Execute search
python ssot/api/search_api.py < request.json > results.json

# 3. Process results (jq example)
cat results.json | jq '.results[] | {id: .doc_id, score: .score}'
```

---

## Simulation - Physics Framework

### Available Engines:
- `igsoa_complex` - 1D complex field dynamics
- `igsoa_complex_2d` - 2D complex field
- `igsoa_complex_3d` - 3D complex field
- `igsoa_gw` - Gravitational wave engine
- `satp_higgs_1d` - 1D Higgs dynamics
- `satp_higgs_2d` - 2D Higgs dynamics
- `satp_higgs_3d` - 3D Higgs dynamics
- `phase4b` - DASE phase 4B

### Command Schema:
```json
{
  "command": "string (required)",
  "params": {
    // Command-specific parameters
  }
}
```

### Response Schema:
```json
{
  "command": "string",
  "status": "success|error",
  "execution_time_ms": number,
  "result": {
    // Command-specific result
  },
  "error": {
    "code": "string",
    "message": "string",
    "details": {}
  }
}
```

### Common Commands:

**List Engines:**
```json
{"command": "list_engines"}
```

**Create Engine:**
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

**Evolve System:**
```json
{
  "command": "evolve",
  "params": {
    "timesteps": 100,
    "dt": 0.01
  }
}
```

**Get State:**
```json
{
  "command": "get_state",
  "params": {
    "format": "json"
  }
}
```

**Snapshot:**
```json
{
  "command": "snapshot",
  "params": {
    "output_file": "snapshot.json"
  }
}
```

### Example Workflow:
```bash
# Create mission file
cat > mission.json << 'EOF'
{"command":"create_engine","params":{"engine_type":"igsoa_complex","num_nodes":1024,"R_c":1.5}}
{"command":"evolve","params":{"timesteps":50,"dt":0.01}}
{"command":"get_state","params":{"format":"json"}}
EOF

# Run simulation (processes all commands)
Simulation/bin/dase_cli.exe < mission.json > results.json

# Extract state
cat results.json | jq 'select(.command == "get_state") | .result'
```

---

## Integration Example

### Query SSOT → Configure Simulation → Run

```bash
#!/bin/bash

# 1. Search SSOT for relevant knowledge
echo '{
  "query": "igsoa coupling parameters",
  "mode": "full_text",
  "limit": 10
}' | python ssot/api/search_api.py > workspace/output/ssot_results/query_001.json

# 2. Extract parameters (agent processing)
# ... analyze results and determine simulation parameters ...

# 3. Create simulation based on knowledge
echo '{
  "command": "create_engine",
  "params": {
    "engine_type": "igsoa_complex",
    "num_nodes": 4096,
    "R_c": 2.0
  }
}
{"command":"evolve","params":{"timesteps":100,"dt":0.01}}
{"command":"get_state","params":{"format":"json"}}' > workspace/input/mission_001.json

# 4. Run simulation
Simulation/bin/dase_cli.exe < workspace/input/mission_001.json > workspace/output/simulation_results/run_001.json

# 5. Analyze results
cat workspace/output/simulation_results/run_001.json | jq '.result'
```

---

## File Locations

### SSOT:
- **API:** `ssot/api/search_api.py`
- **Schemas:** `ssot/api/schemas/*.schema.json`
- **Config:** `ssot/config.json`
- **Data:** `ssot/data/ssot_parallel.db` (1.5GB)

### Simulation:
- **CLI:** `Simulation/bin/dase_cli.exe`
- **Reference:** `Simulation/docs/api-reference/`
- **Config:** `Simulation/context.json`

### Workspace:
- **Input:** `workspace/input/` (stage requests)
- **Output:** `workspace/output/` (results storage)
  - `ssot_results/` - SSOT search results
  - `simulation_results/` - Simulation outputs

---

## Schema Validation

All JSON can be validated:

```python
import json
import jsonschema

# Load schema
with open('ssot/api/schemas/search_request.schema.json') as f:
    schema = json.load(f)

# Validate request
request = {"query": "test", "mode": "full_text"}
jsonschema.validate(instance=request, schema=schema)
```

Install validator:
```bash
pip install jsonschema
```

---

## Error Handling

Both components return errors in consistent format:

```json
{
  "status": "error",
  "error": {
    "code": "ERROR_CODE",
    "message": "Human-readable description"
  }
}
```

### Common SSOT Errors:
- `EMPTY_QUERY` - Query string is empty
- `DATABASE_ERROR` - SQLite error
- `INVALID_JSON` - Malformed input

### Common Simulation Errors:
- `INVALID_COMMAND` - Unknown command
- `INVALID_PARAMS` - Missing/invalid parameters
- `ENGINE_ERROR` - Engine creation/execution failed

---

## Performance

### SSOT Search:
- Typical query: ~180ms for 5 results
- Database: 1.5GB (fully indexed)
- Concurrent safe: Yes (SQLite read-only)

### Simulation:
- Engine creation: ~45ms
- Evolution: Depends on timesteps/nodes
- Concurrent: Multiple instances can run

### Optimization Tips:
1. Use appropriate SSOT sensitivity (lower = faster)
2. Limit SSOT results to needed amount
3. Enable simulation cache for repeated runs
4. Use batch commands for simulation workflows

---

## Configuration

### Global: `environment.json`
```json
{
  "airs": {
    "version": "1.0.0",
    "root_path": "D:/airs",
    "components": {
      "ssot": {"enabled": true},
      "simulation": {"enabled": true}
    }
  }
}
```

### SSOT: `ssot/config.json`
- Search backends (FTS5, Whoosh, C++)
- Default sensitivity
- Audit settings

### Simulation: `Simulation/context.json`
- Default engine
- Cache settings
- Output format

### Central config set: `config/`
- Phase sets, stress/wrapper configs, harness configs, and quarantine lists now live under `config/`.

---

## Validation

Test entire setup:
```bash
python scripts/validate_structure.py
```

Expected: All 46 checks pass

### Validation Suite (Phase 5)
Run scenario-based validation (outputs JSON evidence to `artifacts/validation/`):
```bash
python validation/run_validation.py --problem_id diffusion_1d_fixed
# or
python validation/run_validation.py --all
```
Rollup summary: `validation/reports/summary.json`
Optional engine comparison: place engine metrics in `validation/engine_outputs/<problem_id>.json` (e.g., `mass_end`, `variance_slope`, `max_flow`, `max_imbalance`, `max_radius`, `bounded`) to have the harness judge them against reference tolerances.

### CLI Usage Notes (observed)
- PowerShell piping: `Get-Content mission.jsonl | .\Simulation\bin\dase_cli.exe > output.jsonl` (one JSON command per line).
- Working commands: `create_engine` → `set_igsoa_state` (`profile_type` such as `gaussian`) → `run_mission` → `get_state` → `destroy_engine`.
- `evolve` command is not recognized in current CLI (returns `UNKNOWN_COMMAND`).
- `engine_id` must be present in `get_state`, `run_mission`, `destroy_engine`.
- Mission pattern that ran cleanly: create_engine (igsoa_complex, e.g., num_nodes=512, R_c=1.0, dt optional) → set_igsoa_state gaussian → get_state (baseline) → run_mission (num_steps, iterations_per_node) → get_state (final) → destroy_engine.

---

## Common Agent Patterns

### Pattern 1: Knowledge-Driven Simulation
```
Query SSOT → Extract parameters → Configure sim → Run → Analyze
```

### Pattern 2: Iterative Refinement
```
Run sim → Analyze → Query SSOT for optimization → Adjust → Re-run
```

### Pattern 3: Batch Processing
```
Create N variations → Run all in parallel → Collect results → Compare
```

### Pattern 4: Knowledge Extraction
```
Query SSOT → Extract docs → Process → Store results → Index new knowledge
```

---

## Troubleshooting

**SSOT returns no results:**
- Try higher sensitivity (2 or 3)
- Use `mode: "keyword"` for exact match
- Check query sanitization (special chars removed)

**Simulation command fails:**
- Validate JSON syntax
- Check reference: `Simulation/docs/api-reference/`
- Verify engine name in `list_engines` output

**Performance slow:**
- SSOT: Lower sensitivity, reduce limit
- Simulation: Enable cache, reduce nodes/timesteps

**Path errors:**
- All paths relative to component root
- Check `environment.json` for root path
- Use absolute paths in scripts

---

## Resources

- **Product/System Doc:** `docs/guides/PRODUCT_DOCUMENTATION.md`
- **CRF Reference:** `crf/summary.md` (HTML at `crf/Constraint_Resolution_Framework.html`)
- **Component READMEs:** `ssot/README.md`, `Simulation/README.md`
- **Config Sets:** `config/`
- **Reports/Reviews:** `docs/reports/`, `docs/reviews/`

---

## Summary for Agents

✅ **Two standalone JSON components**
✅ **Strict schemas for validation**
✅ **Fully tested and operational**
✅ **2.3GB knowledge base ready**
✅ **Multiple physics engines available**
✅ **Production-ready for autonomous operation**

**Start using immediately - all systems operational.**
