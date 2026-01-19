# Code Review Report

Scope: Code-only review for errors and breaking issues; assess production status.

## Findings (ordered by severity)

1) Critical: Potential out-of-bounds read when mixer length does not match SSP field length.
   - `sum_field` iterates `len` provided by mixer without validating against SSP length.
   - Risk: out-of-bounds read if SSP field_len differs from mixer field_len.
   - Location: `src/sid_mixer.c` lines 29-33, 106-110.

2) High: Dual-mask constraint is computed but not enforced.
   - `mask_sum` is clamped but unused; `delta_I` and `delta_N` still use raw masks.
   - Risk: M_I + M_N > 1 can over-collapse or violate ternary invariants.
   - Location: `src/sid_semantic_processor.c` lines 197-205.

3) Medium: `sid_ssp_add_mass` accepts negative mass and clamps per-cell to zero.
   - Risk: silent loss of mass and conservation violation without error signaling.
   - Location: `src/sid_semantic_processor.c` lines 108-119.

## Production Status

Not production-ready. This is a prototype: missing strict length contracts, mask constraint enforcement, and tests to verify conservation and collapse invariants.

## Open Questions / Assumptions

- Are SSPs and Mixer always created from the same configuration path that guarantees identical `field_len`?
- Should `alpha` and masks be unitless fractions in all paths, or is the current mass-scaled formulation intentional?
