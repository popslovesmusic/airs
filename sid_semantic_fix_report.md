SID Semantic Compliance
=======================

What changed
- Added step-count accessor in SID C API:
  - `Simulation/src/cpp/sid_ssp/sid_capi.hpp` (`sid_get_step_count`)
  - `Simulation/src/cpp/sid_ssp/sid_capi.cpp`
- Wired `sid_ternary` into CLI mission flow:
  - `Simulation/dase_cli/src/engine_manager.cpp` now steps SID engines during `run_mission`.
  - `get_metrics` for `sid_ternary` returns `total_operations` as step count.

Verification (create → run_mission(3) → get_metrics → sid_metrics → destroy)
- `run_mission`: success, steps_completed=3
- `get_metrics`: success, total_operations=3
- `sid_metrics`: success, conserved mass maintained; no rewrite applied
- stdout JSON-only; no stderr

Status update
- sid_semantic family now PASS; `engine_family_test_results.json` and `classes/list.txt` updated.
