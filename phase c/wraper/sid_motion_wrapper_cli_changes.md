sid_motion_wrapper CLI/API changes (proposal)
=============================================

Goal
----
Expose enough signals from `sid_ternary` for an external “semantic motion” wrapper to consume rewrite events and apply mass deltas, without altering the base engine’s symbolic-only semantics.

Required new commands/endpoints
-------------------------------
- `sid_rewrite_events` (read-only, append-only)
  - Params: `engine_id`, optional `cursor` (opaque), optional `limit` (default 100).
  - Returns: `events` array of `{event_id, rule_id, applied (bool), message, timestamp, metadata}` and `next_cursor`.
  - Purpose: wrapper polls this to detect motion-eligible rewrites.
- `sid_rule_metadata` (read-only)
  - Params: `engine_id`, `rule_id`.
  - Returns: metadata for the rule, including `mode` (`symbolic_only` | `semantic_motion`) and optional `epsilon`.
  - Purpose: wrapper decides whether to apply motion and what epsilon to use.
- `sid_wrapper_metrics` (computed)
  - Params: `engine_id` (wrapper-managed logical id).
  - Returns: wrapper mass state `{I_mass, N_mass, U_mass, is_conserved, motion_applied_count, last_motion}` for observability.

Minor extensions to existing commands
-------------------------------------
- `sid_rewrite` and `sid_set_diagram_expr/json`:
  - Accept optional `rule_metadata` object (e.g., `{"mode":"semantic_motion","epsilon":0.05}`) to flow through to the event stream.
  - Preserve current behavior when metadata is absent (symbolic-only).

Wrapper responsibilities (external)
-----------------------------------
- Poll `sid_rewrite_events` with cursor.
- For events where `mode == semantic_motion`:
  - Guard: require `U_mass >= epsilon` using latest `sid_metrics` (base engine) or cached wrapper state.
  - Apply mass deltas in wrapper state: `I += epsilon; U -= epsilon; N unchanged`.
  - Enforce conservation/non-negativity; if guard fails, record `applied=false` and no mass change.
- Expose wrapper state via `sid_wrapper_metrics`.

Non-goals / invariants
----------------------
- Do **not** mutate base engine masses or diagram from the wrapper.
- Default rewrites remain symbolic-only unless `mode` is explicitly set to `semantic_motion`.
- Mass motion must only occur via wrapper; base `sid_metrics` remains the source of truth for diagram-level conservation.

Implementation sketch (minimal C++ work)
----------------------------------------
1) Instrument `sid_ternary` rewrites to append events (rule_id, applied, message, optional metadata) to an in-memory vector; expose via `sid_rewrite_events` with cursor.
2) Allow passing `rule_metadata` through `sid_rewrite` so the event stream carries `mode`/`epsilon`.
3) Add thin CLI handlers for `sid_rewrite_events`, `sid_rule_metadata` (if rules are pre-registered), and `sid_wrapper_metrics` (wrapper can be in-process or out-of-process; in-process version holds wrapper state keyed by engine_id).

Validation once hooks exist
---------------------------
- Symbolic-only rewrite: masses unchanged in wrapper; `sid_rewrite_events` shows applied event with `mode` absent or `symbolic_only`.
- Semantic-motion rewrite: wrapper state shows `I` increased and `U` decreased by `epsilon`; conservation holds.
- Guard refusal: when `U < epsilon`, wrapper records `applied=false` and masses unchanged.

