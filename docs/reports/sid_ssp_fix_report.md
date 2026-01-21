# SID SSP Engine - CLI Integration Report

## Summary
- Added a dedicated SID SSP engine wrapper to the CLI.
- Wired create/run/get_metrics/get_state/destroy for engine_type `sid_ssp`.
- Validated the CLI path with a single-step mission run.

## Implementation Notes
- `sid_ssp` uses the SSP C API via a small RAII wrapper that tracks steps and total operations.
- `capacity` (or `semantic_capacity`) maps to `R_c` for CLI parameters; `role` defaults to 2 (U).

## Validation
- CLI JSON sequence: create_engine (sid_ssp, num_nodes=16) -> run_mission(1) -> get_metrics -> destroy_engine.
- Results: all commands returned status=success and metrics reported total_operations=16.

## Files Updated
- D:\airs\Simulation\dase_cli\src\engine_manager.h
- D:\airs\Simulation\dase_cli\src\engine_manager.cpp
- D:\airs\Simulation\dase_cli\src\command_router.cpp
- D:\airs\engine_family_test_results.json
- D:\airs\classes\list.txt
