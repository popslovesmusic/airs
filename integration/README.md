# Integration - Cross-Component Workflows

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
