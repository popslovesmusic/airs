# AIRS Validation Suite (Phase 5)

Operationalizes the validation problems outlined in `workspace/phase5.txt`. Each scenario is evidence-driven: JSON in, JSON out, with invariants and metrics logged per run.

## Scenarios (catalog)
- `diffusion_1d_fixed` — mass-conserving diffusion/heat equation; checks L1 error and drift.
- `random_walk_to_diffusion` — random walk converging to diffusion; checks variance slope and distribution fit.
- `graph_flow_conservation` — flow on small graphs; checks conservation and cut correctness.
- `lorenz_invariant_measure` — chaos sanity; checks boundedness and attractor shape.

See `validation/catalog.json` for the authoritative list.

## Schema
- Results must follow `validation/validation.schema.json`.
- Required fields: `problem_id`, `timestamp_utc`, `verdict`, `observations`.
- Valid verdicts: `pass`, `fail`, `passes_within_tolerance`, `not_run`, `not_implemented`.

## Harness
- Entry point: `python validation/run_validation.py --problem_id diffusion_1d_fixed`
  - `--all` runs every scenario in the catalog.
  - Outputs JSON to `artifacts/validation/<problem_id>_<timestamp>.json`.
- Runners are implemented in pure Python reference form (no engine calls yet); integrate engine outputs as needed.
- Optional engine comparison: drop engine-produced metrics at `validation/engine_outputs/<problem_id>.json` (e.g., `mass_end`, `variance_slope`, `max_flow`, `max_imbalance`, `max_radius`, `bounded`). The harness will compare them against reference expectations and fail the verdict if they exceed tolerances.

## Implementation notes
- Reference implementations:
  - Diffusion: simple finite-difference solver for baseline; compare engine output to reference over timesteps.
  - Random walk: Monte Carlo to get variance slope; compare to engine diffusion behavior.
  - Graph flow: small graphs with known cuts; check flow conservation and cut correctness.
  - Lorenz: compare boundedness and qualitative attractor shape (not trajectories).
- Invariants to log: conservation, boundedness, variance slope, imbalance, drift.

## Reporting
- Per-run outputs live in `artifacts/validation/`.
- Rollup reports can be aggregated into `validation/reports/` (not yet created).

## Next steps
- Implement the runners in `validation/run_validation.py`.
- Add CI-friendly lightweight subset (diffusion + random_walk) to build scripts.
- Extend docs with any new scenarios; keep catalog and schema in sync.
