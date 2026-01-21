SID Wrapper Integration Report
=============================

Overview
--------
- Added an external semantic-motion wrapper for SID, wired into `dase_cli`, and built a SID-only CLI (`sid_cli`) that uses the same wrapper semantics.
- Base `sid_ternary` remains symbolic-only; wrapper mass motion is opt-in and external.

Key changes
-----------
- CommandRouter:
  - New commands: `sid_rewrite_events`, `sid_wrapper_apply_motion`, `sid_wrapper_metrics`.
  - `sid_rewrite` now accepts optional `rule_metadata` (e.g., `{mode:"semantic_motion", epsilon:0.05}`) and records events.
- EngineManager:
  - Records SID rewrite events (rule_id, applied, message, metadata, timestamp).
  - Maintains wrapper state per SID engine (wrapper-owned I/N/U masses, motion counters, event cursor, last_motion).
  - Applies wrapper motion on demand via `sidWrapperApplyMotion` (guard U >= epsilon; epsilon from metadata or fallback alpha).
- New SID-only CLI:
  - Source: `wrapper/sid_cli.cpp`; target `sid_cli`; binary at `Simulation/bin/sid_cli.exe`.
  - Provides SID-only commands with the same wrapper semantics.

Usage (dase_cli)
----------------
1) Create engine:
```json
{"command":"create_engine","params":{"engine_type":"sid_ternary","num_nodes":8,"alpha":0.1}}
```
2) Seed diagram (ensure rewrite applicability):
```json
{"command":"sid_set_diagram_expr","params":{"engine_id":"engine_001","expr":"X"}}
```
3) Issue semantic-motion rewrite (opt-in via metadata):
```json
{"command":"sid_rewrite","params":{"engine_id":"engine_001","pattern":"X","replacement":"Y","rule_id":"rw1","rule_metadata":{"mode":"semantic_motion","epsilon":0.05}}}
```
4) Apply motion in wrapper (process events):
```json
{"command":"sid_wrapper_apply_motion","params":{"engine_id":"engine_001"}}
```
5) Inspect wrapper mass state:
```json
{"command":"sid_wrapper_metrics","params":{"engine_id":"engine_001"}}
```
6) (Optional) View event log:
```json
{"command":"sid_rewrite_events","params":{"engine_id":"engine_001","cursor":0,"limit":100}}
```
7) Destroy:
```json
{"command":"destroy_engine","params":{"engine_id":"engine_001"}}
```

Semantics and guards
--------------------
- Motion applies only when:
  - `rule_metadata.mode == "semantic_motion"` (or `mode` in params) AND rewrite `applied == true`.
  - Guard: wrapper U_mass >= epsilon (epsilon from metadata or fallback alpha); else motion is refused (last_motion reason `guard_failed`).
- Mass motion: I += epsilon, U -= epsilon, renormalize; conservation checked in wrapper.
- Symbolic rewrites (no mode, or `symbolic_only`) never change wrapper mass_state.

SID-only CLI (sid_cli) commands
-------------------------------
- `sid_create`, `sid_destroy`
- `sid_metrics`
- `sid_set_diagram_expr`, `sid_set_diagram_json`, `sid_get_diagram_json`
- `sid_rewrite` (mode/epsilon via params -> rule_metadata)
- `sid_rewrite_events`
- `sid_wrapper_apply_motion`, `sid_wrapper_metrics`
- `sid_run` (for sid_ssp commit steps)

Build artifacts
---------------
- `Simulation/bin/dase_cli.exe` (updated with wrapper handlers)
- `Simulation/bin/sid_cli.exe` (SID-only adapter)

Notes
-----
- Base `sid_ternary` behavior is unchanged (diagram-only rewrites). All mass motion is wrapper-owned and must be triggered via `sid_wrapper_apply_motion`.
- Guards are wrapper-only; engine does not enforce mass thresholds.
