# Integration Recommendation (Agent-First)

## Summary

Integrate SSP into DASE. The DASE CLI already provides a JSON command surface for agents; embedding SSP there avoids duplicating orchestration and exposes a stable control plane.

## Why DASE Integration Fits Agents

- Existing JSON command router (`simulation/dase_cli`) already supports engine lifecycle management.
- Backend schema already understands engine commands, enabling agent-driven workflows.
- Avoids creating a parallel orchestration layer for SID/SSP.

## Current Blocks

- IGSOA/DASE C-APIs do not expose zero-copy field pointers; SSP needs efficient field access for metrics and collapse.
- DASE engine API is mission-oriented and lacks direct field buffer access.

## Minimal Integration Path

1) Add a new engine type in DASE CLI (e.g., `sid_ternary`).
2) Have that engine type own three engine handles (I/N/U) plus a Mixer instance.
3) Extend C-APIs to expose field buffers (or allow SSP-owned output buffers).
4) Add SID commands in CLI (`sid_step`, `sid_collapse`, `sid_metrics`) for agent control.

## Standalone SSP

Keep the current SID demo only as a development harness. For agent workflows, DASE integration is the clean path.
