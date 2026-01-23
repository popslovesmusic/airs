# Phase 5 Validation Suite Status

**Date:** 2026-01-21  
**Scope:** Operationalizing the validation problems from `workspace/phase5.txt` and producing evidence artifacts.

## What was added
- `validation/validation.schema.json` — schema for validation outputs.
- `validation/catalog.json` — scenario list (diffusion, random walk, graph flow; optional Lorenz).
- `validation/run_validation.py` — runnable harness with reference implementations for three scenarios (Lorenz stubbed).
- `validation/README.md` — usage, scenarios, and notes.
- Artifacts directory: `artifacts/validation/` (JSON evidence per run).
 - Optional engine outputs: drop metrics in `validation/engine_outputs/<problem_id>.json` to compare against references.

## Current scenario status
- `diffusion_1d_fixed`: Reference FTCS diffusion on [0,1], mass conservation checked. **Latest Verdict:** **fail** (engine mass drift vs reference; see latest artifact/summary).
- `random_walk_to_diffusion`: Variance slope of unbiased walk vs expected linear growth. **Verdict:** passes_within_tolerance.
- `graph_flow_conservation`: Edmonds–Karp on small graph with known max flow; conservation checked. **Verdict:** passes_within_tolerance.
- `lorenz_invariant_measure`: RK4 Lorenz integration; boundedness/attractor sanity. **Verdict:** passes_within_tolerance.

## How to run
```bash
python validation/run_validation.py --all
# or single scenario
python validation/run_validation.py --problem_id diffusion_1d_fixed
```
Outputs land in `artifacts/validation/`.

## Latest artifacts (sample)
- `artifacts/validation/diffusion_1d_fixed_20260122T035050.705088+0000.json` (verdict: fail; engine mass drift recorded)
- `artifacts/validation/random_walk_to_diffusion_20260122T035050.892135+0000.json`
- `artifacts/validation/graph_flow_conservation_20260122T035050.892135+0000.json`
- `artifacts/validation/lorenz_invariant_measure_20260122T035050.907706+0000.json`
- Summary rollup: `validation/reports/summary.json` (fail count reflects diffusion engine drift)

## Next steps
1) Integrate Simulation engine outputs by emitting metrics into `validation/engine_outputs/<problem_id>.json` and letting the harness compare to references (mass drift, variance slope, flow imbalance, Lorenz boundedness).
2) Wire a lightweight CI run (diffusion + random_walk) and keep `validation/reports/summary.json` updated.
3) Record any design decisions/constraints in `docs/reports/` and keep catalog/schema in sync with scenario changes.
