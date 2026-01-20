# Simulation Code Review – Stubs & Wiring Risks

Findings are ordered by severity. Paths are workspace-relative.

## High
- **Resolved** `Simulation/dase_cli/src/command_router.cpp` (handleRunMission): Returns real `total_operations` from engine metrics, falling back to the legacy estimate only if metrics are zero.

## Medium
- **Resolved** `Simulation/src/cpp/sid_ternary_engine.hpp` (setDiagramJson/getDiagramJson): Implemented JSON load/save for diagrams using the Diagram model (nodes/edges round-trip).
- **Resolved** `Simulation/src/cpp/sid_ssp/sid_mixer.hpp` (request_collapse): Collapse now redistributes U→I/N with metric recomputation instead of a uniform stub mask.

## Low
- **Resolved** `Simulation/dase_cli/src/command_router.cpp` (handleRunBenchmark): Executes runMission and returns metrics instead of a stub response.
- `Simulation/dase_cli/src/engine_manager.cpp` (setNodeState/getNodeState ~775): Added set/get with field selection for igsoa_complex (1/2/3D) and SATP Higgs (phi, phi_dot, h, h_dot). Benchmark warnings about size_t→int remain; safe but noisy.

## Notes
- GW, SID, SATP, FFTW engines load and run via CLI; `run_mission` and `run_benchmark` now report real ops where available.
