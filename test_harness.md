# Test Harness Implementation (Jan 23, 2026)

## What’s in place
- GTest + CTest harness covers DASE engines (basic_compute_substrate, satp_higgs, igsoa_gw, igsoa_complex) and SID engines (sid_ssp, sid_ternary).
- Engine CLIs are exercised through lightweight C++ step runners (dase_step_runner.exe, sid_step_runner.exe) that pipe fixture JSONL commands into the CLI and capture stdout.
- Runners now use CreateProcessW with pipes (no PowerShell) and normalize volatile fields (engine_id, execution_time_ms, perf numbers) before hashing so hashes are repeatable.
- Harness policy table still loaded from alidation table.txt; unknown engines default to safe/forbidden.
- Metrics are saved under metrics/<engine>/...json for each test run.
- Goldens (hash-only) live under rtifacts/validation/<engine>/out.json and are used by tests.

## Current golden hashes
- basic_compute_substrate: be8b580ae633901
- satp_higgs: 14650fb0739d0383
- gw: 268030760406aaac
- igsoa_complex: c2b53c7df81da87
- sid_ssp: 14650fb0739d0383
- sid_ternary: 14650fb0739d0383

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

## Adding a new engine family
1) Put CLI in Simulation/dase_cli/<your_cli>.exe or add a new runner if protocol differs.
2) Add a fixture JSONL under Simulation/tests/fixtures/inputs/ with create_engine + un_steps commands.
3) Run the appropriate step runner to generate a golden hash into rtifacts/validation/<engine>/out.json.
4) Add a harness file in Simulation/tests/harness/ following the existing patterns and assert the hash.
5) Rebuild harness_tests and commit the new golden + metrics directory if needed.

## No-fallback policy
- Runner missing, CLI errors, non-zero exit codes, or hash mismatches now fail the test (no auto-pass or fallback).

## Known open items
- Placeholder tests for determinism/vacuum/consistency remain TODO.
- Clean up Testing/Temporary scratch folder if not needed.
