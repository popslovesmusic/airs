## Runtime Authority — AIRS CLI

- `Simulation/dase_cli/` is the single authoritative runtime bundle for AIRS CLI binaries.
- Use only the executables in this directory (`dase_cli.exe`, `sid_cli.exe`) for governed and lab runs.
- `sid_cli` is a lab tool (engine-facing); it is **not** an ingress path for governed workflows.
- Diagram semantics are canonicalized internally; sequencing and governance live above the gate.
