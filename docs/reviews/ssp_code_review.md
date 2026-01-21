SSP CODE REVIEW (INTEGRATION FOCUSED)

Scope
- Review target: D:\airs\ssp
- Integration context: D:\airs\Simulation is the canonical project; SSP should integrate as a library or module without correctness drift or platform incompatibilities.

Findings (ordered by severity)

1) Conservation metrics are computed using stale I/N values after routing
- File: D:\airs\ssp\src\sid_mixer.c:114-170
- Issue: `sid_mixer_step` routes mass from U into I/N, then recomputes only U and uses the old I/N values to rebuild `total` and later to set metrics. This can make `total` (and conservation_error) incorrect and can skip scaling even when total mass still exceeds C.
- Impact: Conservation monitoring and transport readiness can be wrong; downstream integration may incorrectly accept non-conserved states.

2) Loop gain denominator ignores sign, yielding extreme values when U increases
- File: D:\airs\ssp\src\sid_mixer.c:193-196
- Issue: `denom = maxd(dU, 1e-12)` uses the signed delta; when `dU` is negative (U increased), denom collapses to 1e-12, producing huge loop_gain.
- Impact: Spurious instability signals and misleading telemetry in integrated runs; transport predicate may be distorted.

3) Route-from-field relies on asserts for bounds safety
- File: D:\airs\ssp\src\sid_semantic_processor.c:183-203
- Issue: `src_len` is only checked via `assert`; in release builds it is ignored, but the loop still uses `ssp->field_len`, risking out-of-bounds reads if callers pass mismatched buffers.
- Impact: Potential memory corruption when SSP is integrated into external engines without debug assertions.

4) Build system hard-requires AVX2
- File: D:\airs\ssp\CMakeLists.txt:22-27
- Issue: `-mavx2`/`/arch:AVX2` is always enabled. This will fail to build or crash at runtime on CPUs without AVX2.
- Impact: Integration into mixed hardware fleets (or CI) becomes fragile; should be optional or gated.

Integration Notes
- For integration with Simulation, consider exporting SSP as a library target (not just the demo executable) so it can be linked cleanly.
- Consider aligning runtime metrics with Simulation?s profiling pipeline to keep telemetry consistent across components.

End of report.
