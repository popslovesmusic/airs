# Test Harness Implementation (Jan 23, 2026)

## What’s in place
- GTest + CTest harness covers DASE engines (basic_compute_substrate, satp_higgs, igsoa_gw, igsoa_complex) and SID engines (sid_ssp, sid_ternary).
- Engine CLIs are exercised through lightweight C++ step runners (dase_step_runner.exe, sid_step_runner.exe) that pipe fixture JSONL commands into the CLI and capture stdout.
- Runners use CreateProcessW with pipes (no PowerShell) and normalize volatile fields (engine_id, execution_time_ms, perf numbers) before hashing so hashes are repeatable.
- Harness policy table still loaded from alidation table.txt; unknown engines default to safe/forbidden.
- Metrics are saved under metrics/<engine>/...json for each test run.
- Goldens now include a single deterministic metric per engine in rtifacts/validation/<engine>/out.json.

## Current goldens (hash + metric)
- basic_compute_substrate: hash 956aeb0623414a47, state_norm=1024
- satp_higgs: hash 14650fb0739d0383, state_norm=0
- gw: hash 8813f85bb44d6ef, state_norm=1.45005e-87
- igsoa_complex: hash 1ecbfa6e26f8fff, state_norm=32
- sid_ssp: hash 8d447646765728c0, active_nodes=1024
- sid_ternary: hash 956d1239323d716f, active_nodes=1024

## How to run
1) Build (Debug shown; use Release similarly):
   `powershell
   cmake --build build --config Debug --target harness_tests
   `
2) Execute harness suite:
   `powershell
   cd build
   ctest -C Debug -L harness --output-on-failure
   `
   or directly: uild/Debug/harness_tests.exe.

## Metrics & no-fallback
- One engine → one metric → one assertion.
  - DASE: metrics.state_norm (L2 norm of primary state after one step)
  - SID: metrics.active_nodes (count of active nodes/entries)
- Runner missing, CLI errors, non-zero exit codes, or hash/metric mismatches now fail the test (no auto-pass or fallback).

## Adding a new engine family
1) Put CLI in Simulation/dase_cli/<your_cli>.exe or add a new runner if protocol differs.
2) Add a fixture JSONL under Simulation/tests/fixtures/inputs/ with create_engine + un_steps commands.
3) Run the appropriate step runner to generate a golden into rtifacts/validation/<engine>/out.json.
4) Add a harness file in Simulation/tests/harness/ asserting both hash and the single metric.
5) Rebuild harness_tests and commit the updated goldens + metrics as needed.

## Known open items
- Placeholder tests for determinism/vacuum/consistency remain TODO.
- Clean up Testing/Temporary scratch folder if not needed.
