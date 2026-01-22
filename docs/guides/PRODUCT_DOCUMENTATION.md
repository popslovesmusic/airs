# AIRS Product & System Documentation

Current snapshot of the project after recent changes. Historical reports in `docs/` remain available for reference but this document is the active source of truth.

## 1) Product Overview
- **Components**: `ssot/` (knowledge system), `Simulation/` (IGSOA-SIM engines and CLI), `integration/` (cross-component workflows), `workspace/` (shared I/O).
- **Principles**: Strict JSON I/O, separation of concerns (SSOT vs Simulation), immutable source data, version-control ready (artifacts in `artifacts/`).

## 2) System Documentation (Developers)

### Requirements
- Common: Python 3.11+, Git, CMake 3.25+, C/C++ compiler with AVX2, PowerShell 7+ for scripts.
- SSOT: SQLite (FTS5), Whoosh index (shipped in `ssot/data/`), `networkx` for graph utilities, JSON Schema validation.
- Simulation: FFTW3 (binaries/headers in `Simulation/`), CMake presets in `Simulation/CMakeLists.txt`, optional Python tools for cache/profiling.

### Design & Architecture
- **SSOT**
  - Data plane: immutable databases in `ssot/data/` (`ssot_parallel.db`, `rmap_graph.gpickle`, `ssot_index/`).
  - Service plane: JSON search API in `ssot/api/` with schemas under `ssot/api/schemas/`; tools in `ssot/tools/` for CLI search/inspection.
  - Compute plane: AVX2 index/search in `ssot/cpp_index/` built via CMake; binaries in `ssot/bin/`.
  - Integration: all I/O via JSON; audit/logs are gitignored.
- **Simulation (IGSOA-SIM)**
  - Engines: DASE, IGSOA, SATP implemented in C++ under `Simulation/src/` with AVX2 optimizations.
  - CLI: `Simulation/dase_cli/` and `Simulation/cli/` provide headless JSON command processing; configs in `config/` and `Simulation/context.json`.
  - Docs: indexed in `Simulation/docs/INDEX.md` with architecture, API reference, tutorials, and feature status.
  - Artifacts: binaries in `Simulation/bin/`, build tree in `Simulation/build/`, benchmarks in `Simulation/benchmarks/`.

### Design Decisions (highlights)
- Strict JSON contracts between components (schemas versioned under each component).
- Immutable SSOT data; rebuilds go through explicit tools (e.g., `rebuild_index.py`).
- Separation of engines from CLI; CLI only marshals JSON → engine calls → JSON.
- Favor AVX2 builds for performance; fallback CPU path allowed by CMake options.

### Code Logic (high level)
- SSOT search flow: request (validated) → backend selection (SQLite FTS5 or Whoosh) → score/rank → JSON response with snippets/metadata.
- Simulation command flow: JSON command → CLI dispatcher → engine-specific kernel → results/metrics emitted as JSON; optional FFT and cache utilities can pre-warm kernels and wisdom files.

### API Specs (essentials)
- **SSOT Search Request (example)**:
  ```json
  {"query":"gravity waves","mode":"full_text","sensitivity":1,"limit":20}
  ```
  Response: `status`, `query`, `total_results`, `execution_time_ms`, `results[] {doc_id, score, snippet, metadata}`. See `ssot/api/schemas/`.
- **Simulation CLI Command (example)**:
  ```json
  {"command":"list_engines"}
  ```
  Engines accept structured commands per engine (DASE/IGSOA/SATP); refer to `Simulation/docs/api-reference/` for full command/response envelopes.

### Testing Plans
- SSOT: schema validation tests (requests/responses), search regression tests against fixtures, performance sanity on index queries. Target: deterministic results and stable latency.
- Simulation: engine unit tests under `Simulation/tests/`, CLI contract tests with canned JSON commands, performance baselines (benchmarks in `Simulation/benchmarks/`). Target: correctness vs reference outputs plus AVX2 performance checks.
- Cross-component: integration workflows in `integration/` using JSON pipelines; validate against schemas and expected artifacts in `workspace/`.

## 3) User Documentation (Product)
- **Getting Started**
  - SSOT search CLI: `python ssot/tools/search.py "query" --mode full_text --limit 20`
  - Simulation CLI: `cmake -B build -DBUILD_CLI=ON && cmake --build build --config Release && ./dase_cli/dase_cli.exe < commands.json`
- **User Guides & Tutorials**
  - SSOT: see `ssot/README.md` for request/response formats and settings in `ssot/config.json`.
  - Simulation: see `Simulation/docs/getting-started/INSTRUCTIONS.md` and tutorials under `Simulation/docs/command-center/tutorials/`.
- **Reference**
  - API schemas: `ssot/api/schemas/`, Simulation API reference in `Simulation/docs/api-reference/`.
  - Architecture & design: `Simulation/docs/architecture-design/`, SSOT design summary in this doc and `ssot/README.md`.

## 4) How to Maintain This Doc
- Update requirements and architecture sections when adding/removing backends, engines, or build options.
- Keep API examples in sync with schemas and CLI command sets.
- Record design decisions (why) alongside changes; link to change reports in `docs/reports/`.
- Refresh testing plans when new fixtures, benchmarks, or integration workflows are added.

## 5) Additional References
- Constraint Resolution Framework (CRF) notes: see `crf/summary.md` and chapters in `crf/` (build scripts `build_book*.ps1` regenerate `crf/Constraint_Resolution_Framework.html`). Treat as historical guidance; align with current architecture before implementation.

## 6) Constraint Resolution Framework (CRF) Snapshot
- Purpose: conceptual framework for resolving system constraints (performance, correctness, governance). Use it to vet new designs.
- Sources: `crf/summary.md` for an overview; detailed rationale across `crf/chapter_1.md` ... `chapter_28.md`; compiled HTML in `crf/Constraint_Resolution_Framework.html`.
- Regeneration: run `pwsh crf/build_book_v2.ps1` (or `build_book.ps1`) from repo root to rebuild the HTML from chapters.
- Usage guidance:
- Map CRF constraints to current components (SSOT data immutability, Simulation engine safety, JSON contract enforcement).
- When introducing changes, note which CRF constraints are touched and record decisions in `docs/reports/` or an architecture note.
- Prefer lightweight adaptations over wholesale adoption; the CRF is historical and may pre-date recent refactors.

## 7) Validation Suite (Phase 5)
- Location: `validation/` (schema, catalog, harness) with outputs in `artifacts/validation/`.
- Scenarios (see `validation/catalog.json`): `diffusion_1d_fixed`, `random_walk_to_diffusion`, `graph_flow_conservation`, optional `lorenz_invariant_measure`.
- Schema: `validation/validation.schema.json`; required fields include `problem_id`, `timestamp_utc`, `verdict`, `observations`.
- Harness: `python validation/run_validation.py --problem_id <id>` or `--all`; writes JSON evidence to `artifacts/validation/`.
- Status: runners implemented in Python references; integrate Simulation engine outputs as desired. Summary rollup at `validation/reports/summary.json`.
- Engine comparison (optional): drop engine-produced metrics at `validation/engine_outputs/<problem_id>.json` (e.g., `mass_end`, `variance_slope`, `max_flow`, `max_imbalance`, `max_radius`, `bounded`) to have the harness evaluate them against reference tolerances.

## 8) Simulation CLI Usage Notes (observed)
- PowerShell piping: `Get-Content mission.jsonl | .\Simulation\bin\dase_cli.exe > output.jsonl` (each command on a single line).
- Working commands: `create_engine` → `set_igsoa_state` (`profile_type` such as `gaussian`) → `run_mission` → `get_state` → `destroy_engine`.
- `evolve` command is not recognized (returns `UNKNOWN_COMMAND`).
- `engine_id` is required in `get_state`, `run_mission`, and `destroy_engine`.
- Example mission that runs cleanly: create_engine (igsoa_complex, e.g., num_nodes=512, R_c=1.0, dt optional) → set_igsoa_state gaussian → get_state (baseline) → run_mission (num_steps, iterations_per_node) → get_state (final) → destroy_engine.
