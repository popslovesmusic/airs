SATP Higgs Metrics Compliance
=============================

What changed
- Added metrics helpers to SATP Higgs engines to surface `total_updates` as operations and derive `ops_per_sec` / `ns_per_op`:
  - `Simulation/src/cpp/satp_higgs_engine_1d.h`
  - `Simulation/src/cpp/satp_higgs_engine_2d.h`
  - `Simulation/src/cpp/satp_higgs_engine_3d.h`
- Wired metrics retrieval in the active CLI manager:
  - `Simulation/dase_cli/src/engine_manager.cpp` now routes satp_higgs_* to the new helpers.
- Rebuilt `dase_cli.exe` and updated `Simulation/bin`.

Verification (create → run_mission(num_steps=1) → get_metrics → destroy)
- satp_higgs_1d: PASS — total_operations=32, ns_per_op≈312500.0, ops_per_sec≈3.2e3
- satp_higgs_2d: PASS — total_operations=16, ns_per_op≈625000.0, ops_per_sec≈1.6e3
- satp_higgs_3d: PASS — total_operations=27, ns_per_op≈370370.37, ops_per_sec≈2.7e3
- No stderr; stdout JSON-only.

Status update
- satp_higgs family now PASS; `engine_family_test_results.json` and `classes/list.txt` updated.

Notes
- Metrics represent field updates (updates per node per step) per the execution_class rules for field engines; numeric ops are not required, but non-zero progress is now reported.
