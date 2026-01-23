# Repository Guidelines

## Project Structure & Modules
- Core engines and CLI: `Simulation/src/cpp`, `Simulation/dase_cli/src`.
- Tests and harness: `Simulation/tests` (fixtures under `Simulation/tests/fixtures`, goldens under `artifacts/validation`).
- Build outputs: `build/<Config>/` (e.g., `build/Debug`, `build/Release`); top-level executables include `dase_cli.exe`, `harness_tests.exe`, `dase_step_runner.exe`, `sid_step_runner.exe`.
- JSON ingress tools and docs: `json_gate.py`, `docs/json_ingress_gate.md`, quick reference `agent.md`.
- Metrics and validation tables: `metrics/`, `validation/`, `validation table.txt`.

## Build, Test, and Development Commands
- Configure/build (MSVC multi-config shown):  
  `cmake -S . -B build -G "Visual Studio 17 2022" -A x64`  
  `cmake --build build --config Debug`
- Run harness suite:  
  `cd build && ctest -C Debug -L harness --output-on-failure`
- Run SID/DASE step runners directly:  
  `build/Debug/dase_step_runner.exe` or `build/Debug/sid_step_runner.exe`
- CLI describe helper:  
  `build/dase_cli/Release/dase_cli.exe --describe sid_ssp`

## Coding Style & Naming
- C++: follow existing style in `Simulation/*` (brace on same line, 2–4 space indents as per file); prefer `std::` algorithms, avoid raw new/delete.
- Tests: GTest; test files named `test_*.cpp`; harness tests in `Simulation/tests/harness`.
- JSON fixtures: single-object JSONL via `json_gate.py --mode gate`; file names `out_<tag>.json` and `out_<tag>_confirmed.json`.

## Testing Guidelines
- Framework: GoogleTest + CTest labels (`-L harness` for harness subset).
- Determinism: hashes and a single metric per engine must match goldens in `artifacts/validation/<engine>/`.
- No fallback: runner missing, CLI error, or hash/metric mismatch = test failure.
- Add new fixtures under `Simulation/tests/fixtures/inputs/`; update goldens with step runners, then commit both fixture and golden.

## Commit & PR Guidelines
- Messages: short imperative summary (e.g., “Add SID rewrite gate”), include scope when helpful.
- PRs: describe intent, list tests run (`ctest -C Debug -L harness`), mention updated goldens/metrics, attach logs if non-default build configs.

## Security & Configuration Tips
- All JSON ingress must pass `json_gate.py` before hitting `dase_cli.exe`; reject on any validation error.
- Avoid editing generated `build/` artifacts; commit only source, fixtures, goldens, and docs.
- Keep new scripts ASCII unless existing file uses Unicode.***
