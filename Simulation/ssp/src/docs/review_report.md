Findings (code only), ordered by severity

- Conservation correction can fail when excess > U. In sid_mixer.c, the over-capacity path clamps alpha to 1, so if total - C > U, U is zeroed but I+N+U still exceeds C with no further correction. This violates the “hard requirement” global correction. Consider a follow-up correction step that scales I/N or an explicit normalization path. File: sid_mixer.c.
- Correction path removes mass without routing, which breaks collapse semantics and may desync I/N. The over-capacity correction calls sid_ssp_apply_collapse_mask on U only (no routing to I/N), so “collapse” in this path is effectively a sink. If downstream logic assumes collapse always routes to I/N, metrics and state will diverge. File: sid_mixer.c.
- Routing API allows out-of-bounds reads with mismatched source buffers. sid_ssp_route_from_field trusts src_field length to match ssp->field_len and provides no length validation, so callers can pass a shorter buffer and trigger OOB reads. Consider adding a length parameter or a wrapper that routes from another SSP to enforce size matching. File: sid_semantic_processor.c, sid_semantic_processor.h.

Open questions / assumptions
- Should the mixer’s correction path be allowed to scale down I/N (via a new SSP “scale” API), or should it emit an error when excess > U?
- Is it acceptable for conservation correction to bypass I/N routing (treating it as a separate “sink” operation), or do you want all U reduction to be accounted for in I/N?

Change summary
- Review only; no changes made.