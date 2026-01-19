# IGSOA-SIM CLI Usage Guide

Complete guide to using the IGSOA-SIM command-line interface for scientific simulations.

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Basic Concepts](#basic-concepts)
3. [Command Reference](#command-reference)
4. [Example Workflows](#example-workflows)
5. [Advanced Usage](#advanced-usage)
6. [Troubleshooting](#troubleshooting)

---

## Quick Start

### Build the CLI

```bash
# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_CLI=ON
cmake --build build --config Release

# CLI location
build/dase_cli/Release/dase_cli.exe  # Windows
build/dase_cli/dase_cli              # Linux/macOS
```

### Run Your First Simulation

```bash
# Interactive mode (read commands from stdin)
./build/dase_cli/Release/dase_cli.exe

# Batch mode (read from file)
./build/dase_cli/Release/dase_cli.exe < commands.json

# Single command
echo '{"command":"list_engines"}' | ./build/dase_cli/Release/dase_cli.exe
```

---

## Basic Concepts

### JSON-Based Interface

All commands are JSON objects with this structure:

```json
{
  "command": "command_name",
  "params": {
    "param1": "value1",
    "param2": 123
  }
}
```

Responses follow this format:

```json
{
  "command": "command_name",
  "status": "success",
  "execution_time_ms": 1.23,
  "result": { /* command-specific result */ }
}
```

Or for errors:

```json
{
  "command": "command_name",
  "status": "error",
  "error": "Error description",
  "error_code": "ERROR_CODE"
}
```

### Engine Types

| Engine | Description | Dimensions |
|--------|-------------|-----------|
| `igsoa_complex` | Complex-valued IGSOA dynamics | 1D |
| `igsoa_complex_2d` | 2D IGSOA complex field | 2D |
| `igsoa_complex_3d` | 3D IGSOA complex field | 3D |
| `igsoa_gw` | Gravitational wave engine | 3D |
| `satp_higgs_1d` | SATP Higgs 1D dynamics | 1D |
| `satp_higgs_2d` | SATP Higgs 2D dynamics | 2D |
| `satp_higgs_3d` | SATP Higgs 3D dynamics | 3D |
| `phase4b` | DASE phase 4B (production) | 1D |

---

## Command Reference

### 1. Information Commands

#### `list_engines`
List all available engine types.

**Parameters**: None

**Example**:
```json
{"command": "list_engines"}
```

**Response**:
```json
{
  "command": "list_engines",
  "status": "success",
  "result": {
    "engines": [
      "igsoa_complex",
      "igsoa_complex_2d",
      "igsoa_complex_3d",
      "igsoa_gw",
      "satp_higgs_1d",
      "satp_higgs_2d",
      "satp_higgs_3d",
      "phase4b"
    ]
  }
}
```

#### `describe_engine`
Get detailed information about an engine.

**Parameters**:
- `engine_name` (string): Engine type to describe

**Example**:
```json
{"command": "describe_engine", "params": {"engine_name": "igsoa_complex"}}
```

**Response**:
```json
{
  "command": "describe_engine",
  "status": "success",
  "result": {
    "engine": "igsoa_complex",
    "display_name": "IGSOA Complex 1D",
    "description": "1D complex-valued IGSOA dynamics engine",
    "parameters": {
      "R_c": {
        "type": "float",
        "default": 1.0,
        "range": [0.1, 10.0],
        "description": "Coupling constant"
      },
      "num_nodes": {
        "type": "int",
        "default": 1024,
        "range": [64, 16384],
        "description": "Number of spatial nodes"
      }
    },
    "equations": [
      {
        "name": "igsoa_evolution",
        "latex": "\\partial_t \\psi = -i H \\psi",
        "description": "IGSOA field evolution"
      }
    ]
  }
}
```

### 2. Engine Lifecycle Commands

#### `create_engine`
Create a new simulation engine instance.

**Parameters**:
- `engine_type` (string): Type of engine to create
- `num_nodes` (int): Number of spatial nodes
- `R_c` (float, optional): Coupling constant (default: 1.0)
- Additional engine-specific parameters

**Example**:
```json
{
  "command": "create_engine",
  "params": {
    "engine_type": "igsoa_complex",
    "num_nodes": 4096,
    "R_c": 1.5
  }
}
```

**Response**:
```json
{
  "command": "create_engine",
  "status": "success",
  "execution_time_ms": 1.234,
  "result": {
    "engine_id": "engine_001",
    "engine_type": "igsoa_complex",
    "num_nodes": 4096,
    "R_c": 1.5,
    "dt": 0.01,
    "gamma": 0.1,
    "kappa": 1.0
  }
}
```

#### `destroy_engine`
Destroy an engine instance and free resources.

**Parameters**:
- `engine_id` (string): ID of engine to destroy

**Example**:
```json
{"command": "destroy_engine", "params": {"engine_id": "engine_001"}}
```

### 3. Simulation Commands

#### `evolve`
Evolve the simulation forward in time.

**Parameters**:
- `engine_id` (string): Target engine
- `timesteps` (int): Number of timesteps to run
- `dt` (float, optional): Timestep size (default: engine default)

**Example**:
```json
{
  "command": "evolve",
  "params": {
    "engine_id": "engine_001",
    "timesteps": 1000,
    "dt": 0.01
  }
}
```

**Response**:
```json
{
  "command": "evolve",
  "status": "success",
  "execution_time_ms": 234.56,
  "result": {
    "timesteps_completed": 1000,
    "simulation_time": 10.0,
    "final_energy": 1.2345,
    "energy_drift": 0.001
  }
}
```

#### `reset`
Reset engine to initial state.

**Parameters**:
- `engine_id` (string): Target engine

**Example**:
```json
{"command": "reset", "params": {"engine_id": "engine_001"}}
```

### 4. State Access Commands

#### `get_state`
Retrieve current simulation state.

**Parameters**:
- `engine_id` (string): Target engine
- `format` (string, optional): Output format ("json", "binary")
- `fields` (array, optional): Specific fields to retrieve

**Example**:
```json
{
  "command": "get_state",
  "params": {
    "engine_id": "engine_001",
    "format": "json"
  }
}
```

**Response**:
```json
{
  "command": "get_state",
  "status": "success",
  "result": {
    "time": 10.0,
    "num_nodes": 4096,
    "fields": {
      "psi_real": [0.1, 0.2, 0.3, ...],
      "psi_imag": [0.4, 0.5, 0.6, ...]
    },
    "metrics": {
      "energy": 1.2345,
      "momentum": 0.0001
    }
  }
}
```

#### `set_state`
Set simulation state.

**Parameters**:
- `engine_id` (string): Target engine
- `fields` (object): Field data to set

**Example**:
```json
{
  "command": "set_state",
  "params": {
    "engine_id": "engine_001",
    "fields": {
      "psi_real": [0.1, 0.2, 0.3],
      "psi_imag": [0.4, 0.5, 0.6]
    }
  }
}
```

### 5. Analysis Commands

#### `compute_fft`
Compute FFT of field data.

**Parameters**:
- `engine_id` (string): Target engine
- `field` (string, optional): Field to analyze (default: "psi")

**Example**:
```json
{
  "command": "compute_fft",
  "params": {
    "engine_id": "engine_001",
    "field": "psi"
  }
}
```

**Response**:
```json
{
  "command": "compute_fft",
  "status": "success",
  "result": {
    "frequencies": [0.0, 0.1, 0.2, ...],
    "power_spectrum": [1.0, 0.8, 0.6, ...],
    "peak_frequency": 0.5,
    "peak_power": 2.3
  }
}
```

#### `get_metrics`
Retrieve simulation metrics.

**Parameters**:
- `engine_id` (string): Target engine

**Example**:
```json
{"command": "get_metrics", "params": {"engine_id": "engine_001"}}
```

**Response**:
```json
{
  "command": "get_metrics",
  "status": "success",
  "result": {
    "energy": 1.2345,
    "energy_drift": 0.001,
    "momentum": 0.0001,
    "angular_momentum": 0.0002,
    "custom_metrics": {
      "field_norm": 1.0,
      "phase_coherence": 0.95
    }
  }
}
```

---

## Example Workflows

### 1. Basic Simulation

```bash
# Create commands file
cat > simulation.json << 'EOF'
{"command":"create_engine","params":{"engine_type":"igsoa_complex","num_nodes":2048,"R_c":1.0}}
{"command":"evolve","params":{"timesteps":500}}
{"command":"get_metrics"}
{"command":"get_state","params":{"format":"json"}}
{"command":"destroy_engine"}
EOF

# Run simulation
./dase_cli.exe < simulation.json > results.json
```

### 2. Parameter Sweep

```bash
# Sweep over R_c values
for Rc in 0.5 1.0 2.0 5.0 10.0; do
  echo "{\"command\":\"create_engine\",\"params\":{\"engine_type\":\"igsoa_complex\",\"num_nodes\":4096,\"R_c\":$Rc}}" > test_Rc${Rc}.json
  echo "{\"command\":\"evolve\",\"params\":{\"timesteps\":1000}}" >> test_Rc${Rc}.json
  echo "{\"command\":\"get_metrics\"}" >> test_Rc${Rc}.json
  echo "{\"command\":\"destroy_engine\"}" >> test_Rc${Rc}.json

  ./dase_cli.exe < test_Rc${Rc}.json > output_Rc${Rc}.json
  echo "Completed R_c = $Rc"
done
```

### 3. Time Evolution Monitoring

```bash
# Monitor simulation every 100 timesteps
cat > monitor.json << 'EOF'
{"command":"create_engine","params":{"engine_type":"igsoa_complex","num_nodes":1024}}
{"command":"evolve","params":{"timesteps":100}}
{"command":"get_metrics"}
{"command":"evolve","params":{"timesteps":100}}
{"command":"get_metrics"}
{"command":"evolve","params":{"timesteps":100}}
{"command":"get_metrics"}
{"command":"destroy_engine"}
EOF

./dase_cli.exe < monitor.json | grep -A 10 '"get_metrics"'
```

### 4. FFT Analysis Pipeline

```bash
cat > fft_analysis.json << 'EOF'
{"command":"create_engine","params":{"engine_type":"igsoa_complex","num_nodes":8192}}
{"command":"evolve","params":{"timesteps":2000}}
{"command":"compute_fft","params":{"field":"psi"}}
{"command":"get_state","params":{"format":"json","fields":["psi_real","psi_imag"]}}
{"command":"destroy_engine"}
EOF

./dase_cli.exe < fft_analysis.json > fft_results.json
```

---

## Advanced Usage

### Scripting with Python

```python
import subprocess
import json

def run_simulation(engine_type, num_nodes, timesteps, R_c=1.0):
    """Run a simulation and return results."""
    commands = [
        {"command": "create_engine", "params": {
            "engine_type": engine_type,
            "num_nodes": num_nodes,
            "R_c": R_c
        }},
        {"command": "evolve", "params": {"timesteps": timesteps}},
        {"command": "get_metrics"},
        {"command": "destroy_engine"}
    ]

    # Convert to JSON lines
    input_data = "\n".join(json.dumps(cmd) for cmd in commands)

    # Run CLI
    result = subprocess.run(
        ["./dase_cli.exe"],
        input=input_data,
        capture_output=True,
        text=True
    )

    # Parse results
    results = [json.loads(line) for line in result.stdout.strip().split('\n')
               if line.strip() and line.startswith('{')]

    return results

# Use it
results = run_simulation("igsoa_complex", 4096, 1000, R_c=1.5)
metrics = next(r for r in results if r["command"] == "get_metrics")
print(f"Final energy: {metrics['result']['energy']}")
```

### Performance Profiling

```bash
# Measure execution time for different node counts
for N in 1024 2048 4096 8192; do
  echo "Testing N=$N nodes..."
  time (echo "{\"command\":\"create_engine\",\"params\":{\"engine_type\":\"igsoa_complex\",\"num_nodes\":$N}}" | ./dase_cli.exe)
done
```

### Pipeline Integration

```bash
# Use with jq for JSON processing
echo '{"command":"list_engines"}' | ./dase_cli.exe | jq -r '.result.engines[]'

# Extract specific metrics
cat simulation.json | ./dase_cli.exe | jq -r 'select(.command=="get_metrics") | .result.energy'
```

---

## Troubleshooting

### Common Errors

#### "Engine DLL not found"
**Solution**: Ensure DLLs are in the same directory as the executable or in system PATH.
```bash
# Copy DLLs to CLI directory
cp build/Release/*.dll build/dase_cli/Release/
```

#### "Invalid JSON"
**Solution**: Validate JSON syntax before passing to CLI.
```bash
# Validate with jq
cat commands.json | jq . > /dev/null && echo "Valid JSON"

# Or use Python
python -m json.tool commands.json
```

#### "FFTW3 not found"
**Solution**: Ensure libfftw3-3.dll is available.
```bash
# Copy FFTW3 DLL
cp libfftw3-3.dll build/dase_cli/Release/
```

### Debug Mode

To see detailed logging, check stderr output:
```bash
./dase_cli.exe < commands.json 2> debug.log
```

### Performance Tips

1. **Use Release builds**: Always build with `-DCMAKE_BUILD_TYPE=Release`
2. **Enable AVX2**: Ensure `-DENABLE_AVX2=ON` (default)
3. **Batch commands**: Group multiple commands in one file for better performance
4. **Binary I/O**: Use `"format":"binary"` for large state transfers (future feature)

---

## Additional Resources

- **Architecture**: See `docs/implementation/HEADLESS_JSON_CLI_ARCHITECTURE.md`
- **Engine Details**: See `dase_cli/QUICKSTART.txt`
- **Examples**: See `dase_cli/test_*.json` files
- **Migration Guide**: See `CLI_MIGRATION_SUMMARY.md`

---

**Questions?** Check the full documentation at `docs/INDEX.md`
