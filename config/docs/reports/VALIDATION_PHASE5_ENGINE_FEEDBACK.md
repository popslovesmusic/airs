# Phase 5 Validation – Engine Feedback

**Date:** 2026-01-22  
**Scope:** Running engine-backed validation for diffusion_1d_fixed and capturing findings/suggestions.

## Run performed
- Mission: `workspace/mission_diffusion_1d.jsonl`
  - `create_engine` igsoa_complex, num_nodes=512, R_c=1.0
  - `set_igsoa_state` gaussian (amplitude=1.0, center_node=256, width=50)
  - `get_state` (baseline)
  - `run_mission` (num_steps=200, iterations_per_node=20)
  - `get_state` (post-run)
  - `destroy_engine`
- Command: `Get-Content workspace/mission_diffusion_1d.jsonl | .\Simulation\bin\dase_cli.exe > artifacts/validation/diffusion_1d_fixed.raw.jsonl`
- Engine metrics extracted to `validation/engine_outputs/diffusion_1d_fixed.json`.

## Findings (diffusion_1d_fixed)
- Reference runner verdict: would pass (mass drift ~2e-8).
- Engine verdict: **fail** (see latest `artifacts/validation/diffusion_1d_fixed_*.json`):
  - mass_start: ~88.62
  - mass_end: 512.0
  - mass_drift: ~423.38
  - phi_min/max final: ~69.25 / 69.25
  - Drift dominates tolerance; harness marks verdict fail when engine metrics are present.

## Impact on rollup
- `validation/reports/summary.json`: verdict_counts now reflect 1 fail, 3 pass (diffusion fail due to engine drift).

## Suggestions
1) Revisit engine/state setup for diffusion:
   - Check normalization of psi/phi for igsoa_complex in 1D missions.
   - Confirm whether mass scaling to 512 is intended; if so, define acceptable tolerance in engine_output and adjust harness thresholds accordingly.
   - Try smaller timesteps / fewer steps to see drift trend; vary initial profile amplitude/width to match reference scaling.
2) For completeness, generate engine metrics for other scenarios:
   - `random_walk_to_diffusion`: capture variance_slope from an engine-run equivalent and drop into `validation/engine_outputs/random_walk_to_diffusion.json`.
   - `graph_flow_conservation`: if engine supports flow metrics, record max_flow / max_imbalance.
   - `lorenz_invariant_measure`: record bounded/max_radius from engine run.
3) Once engine metrics are in place, rerun `python validation/run_validation.py --all` and regenerate `validation/reports/summary.json`.
4) Optional: set explicit tolerances per scenario if engine outputs operate on different scales than references.

## Files of note
- Engine output: `validation/engine_outputs/diffusion_1d_fixed.json`
- Raw CLI run: `artifacts/validation/diffusion_1d_fixed.raw.jsonl`
- Latest validation artifact: `artifacts/validation/diffusion_1d_fixed_20260122T035050.705088+0000.json`
- Rollup: `validation/reports/summary.json`
