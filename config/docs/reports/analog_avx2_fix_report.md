Analog AVX2 (Phase4b) Compliance Fix
====================================

What changed
- Suppressed mission-loop stdout banners/metrics in `Simulation/src/cpp/analog_universal_node_engine_avx2.cpp` so CLI stdout stays JSON-only.
- Rebuilt binaries (`dase_engine_phase4b.dll` and `dase_cli.exe`) and copied to `Simulation/bin`.

Verification
- Command flow: create_engine (phase4b, num_nodes=128) → run_mission(num_steps=1) → get_metrics → destroy_engine.
- Results (clean JSON, no stderr):
  - run_mission: status=success, steps_completed=1, total_operations=30720.0
  - get_metrics: status=success, engine_type=phase4b, total_operations=3840, ns_per_op≈93.28, ops_per_sec≈1.07e7

Status update
- analog_avx2 family: PASS
- `engine_family_test_results.json` and `classes/list.txt` updated accordingly.
