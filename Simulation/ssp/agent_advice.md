# Agent Advice for SID/DASE Integration

## Core Advice

- Prioritize zero-copy: rely on the extended C-API buffer pointers for direct field access to avoid mission-level copying.
- Handle ambivalence: if `sid_metrics` shows high tension or low `collapse_ratio`, pause and apply resolution rewrites before issuing `sid_collapse`.
- Compartment sync: only invoke Transport (T) after `sid_step` reaches Stability = true and `sid_metrics` confirm stable loop gain and admissible volume thresholds.

## Task.json Alignment

- **Objective**: Manage engine lifecycle and semantic resolution inside `sid_ternary`.
- **Commands**: `sid_step`, `sid_collapse`, `sid_metrics` (zero-copy reads required).
- **Flow**: Initialize `sid_ternary` via DASE CLI, assign I/N/U + Mixer, observe fields, resolve conflicts, collapse only when admissible.

