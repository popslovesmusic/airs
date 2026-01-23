SSP Wrapper Stress Report
=========================

Context
-------
- Task: `stress.json` — stress-test SSP-driven semantic motion under saturation, adversarial inputs, lifecycle churn.
- Constraint: no engine/wrapper logic changes. We added wrapper initialization for `sid_ssp` so wrapper commands work with SSP events.
- Tooling: `dase_cli` with wrapper commands (`sid_rewrite_events`, `sid_wrapper_apply_motion`, `sid_wrapper_metrics`); `run_mission` can record SSP commits when `motion_metadata` is provided.

Implementation notes
--------------------
- `create_engine` for `sid_ssp` now seeds a wrapper mass state (uniform 1/3 each) and an event log, matching `sid_ternary` behavior.
- `run_mission` on `sid_ssp` can include `motion_metadata` and `auto_apply_wrapper_motion`; a `sid_ssp_commit` event is recorded and wrapper motion can be applied immediately.
- Wrapper motion is external and opt-in; base SSP remains unchanged.

Stress test rerun (after SSP wrapper init)
------------------------------------------
- Tracks executed: motion_exhaustion, adversarial_epsilon, commit_storm, lifecycle_reset.
- Outputs:
  - `ssp_wrapper_stress_results.json`
  - `ssp_wrapper_invariant_report.json`
  - `ssp_wrapper_event_counters.json`
- Motion behavior:
  - motion_exhaustion: motions applied: 5; refused: 1 (U mass depleted on last attempt).
  - adversarial_epsilon: motions applied: 0; refused: 1 (guard failed, epsilon > U).
  - commit_storm (symbolic): motions applied: 0; refused: 0; wrapper masses unchanged.
  - lifecycle_reset: success.
- Invariants:
  - No negative masses; wrapper conservation held within tolerance in all tracks.
  - Motion occurred only on semantic_motion metadata; symbolic commits did not move wrapper mass.

Recommended usage
-----------------
- For SSP with motion intent:
```json
{"command":"run_mission","params":{
  "engine_id":"engine_001",
  "num_steps":1,
  "motion_metadata":{"mode":"semantic_motion","epsilon":0.05},
  "auto_apply_wrapper_motion":true
}}
```
- Inspect wrapper mass state after runs:
```json
{"command":"sid_wrapper_metrics","params":{"engine_id":"engine_001"}}
```
- Event log:
```json
{"command":"sid_rewrite_events","params":{"engine_id":"engine_001","cursor":0,"limit":100}}
```

Limitations
-----------
- Wrapper mass state for SSP is wrapper-owned; SSP engine internals remain unchanged.
- Motion application uses epsilon and guard only; no additional SSP-specific redistribution logic is applied beyond wrapper bookkeeping.
