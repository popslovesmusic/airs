Found spec mismatches and risks below, ordered by severity.

- Spec violation: collapse law not implemented in the path the mixer uses. sid_mixer_request_collapse calls sid_ssp_apply_collapse, which subtracts mask[i] * amount (absolute mass), not alpha * (M_I + M_N) * U(x) as required by 03_COLLAPSE_MASKS.md. This breaks the irreversible-collapse semantics and local bounds. Files: sid_mixer.c, sid_semantic_processor.c, 03_COLLAPSE_MASKS.md.
- Spec violation: mixer does not enforce ternary conservation. The mixer only computes conservation_error and never corrects I/N/U to maintain I+N+U=C even though 02_MIXER_SPECIFICATION.md says the mixer corrects globally. This can let violations accumulate indefinitely. Files: sid_mixer.c, 02_MIXER_SPECIFICATION.md.
- Spec mismatch: SSP stability metric not clamped. 01_SSP_SPECIFICATION.md defines stability = 1 - clamp(sum/capacity) but implementation uses 1 - load and allows negative values. That changes the semantics from the spec's [0,1] headroom scale. Files: sid_semantic_processor.c, 01_SSP_SPECIFICATION.md.
- Spec mismatch: transport predicate is incomplete. The mixer uses only the minimal real stability checks, and omits the higher-level predicate criteria (loop_gain convergence, admissible_volume/C >= theta, collapse_ratio monotonic for K steps). If those are required, transport_ready can assert too early. Files: sid_mixer.c, 02_MIXER_SPECIFICATION.md.
- Spec mismatch: collapse bounds not enforced for alpha. sid_ssp_apply_collapse_mask clamps M_I+M_N but allows alpha > 1, which violates the local bounds constraint even if it clamps final delta. This changes the intended control surface. Files: sid_semantic_processor.c, 03_COLLAPSE_MASKS.md.

Open questions / assumptions
- Is the sid_ssp_apply_collapse path intended to remain as a legacy stub, or should sid_mixer_request_collapse be switched to the dual-mask spec path now?
- Is the "mixer corrects globally" statement in 02_MIXER_SPECIFICATION.md a hard requirement for this implementation, or only a future target?
- Are negative stability values acceptable as an overcapacity indicator, or should the implementation adhere strictly to the clamped definition?

Change summary
- No code changes made; this is a spec compliance review of sid_semantic_processor.c, sid_mixer.c, and main.cpp against 01_SSP_SPECIFICATION.md, 02_MIXER_SPECIFICATION.md, and 03_COLLAPSE_MASKS.md.