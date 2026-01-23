Test Harness Plan for `Simulation/src/cpp`
===========================================

Goal
----
- Provide a single, policy-aware harness that runs deterministic, engine-specific checks and emits repeatable evidence to `artifacts/validation/`.

Scope (engines)
---------------
- basic_compute_substrate (foundational numerical kernel)
- satp_higgs (field stability, 1D/2D/3D)
- igsoa_gw (wave propagation)
- igsoa_complex (collapse/attractor engine)
- sid_ssp (semantic signal processor)
- sid_ternary (adjudication/final decision)

Validation policy (from `validation table.txt`)
-----------------------------------------------
- basic_compute_substrate: Allowed → Determinism, Numerical Stability, Performance Scaling, Memory Safety; Forbidden → Diffusion, Conservation, Transport, Semantics.
- satp_higgs: Allowed → Vacuum Stability, Symmetry Breaking, Phase Transition Detection, Boundedness; Forbidden → Mass Conservation, Diffusion Accuracy, Random Walk.
- igsoa_gw: Allowed → Signal Arrival, Echo Structure, Resonance Spectrum, Symmetry Response; Forbidden → Diffusion Convergence, Steady State, Semantic Tests.
- igsoa_complex: Allowed → Attractor Convergence, Constraint Satisfaction, Variance Suppression; Forbidden → Mass Conservation, Diffusion Accuracy, Transport Fidelity.
- sid_ssp: Allowed → Rewrite Determinism, Invariant Preservation, Constraint Satisfaction; Forbidden → Numerical Accuracy, Diffusion, Transport.
- sid_ternary: Allowed → I/N/U Consistency, Determinism, Boundary Handling; Forbidden → Time Evolution, Numerical Dynamics, Variance Analysis.

Harness architecture
--------------------
- Test runner: GoogleTest + CTest targets wired in `Simulation/CMakeLists.txt` (label suites by engine + validation type, e.g., `gw:echo_structure`, `satp:phase_transition`).
- Policy layer: loader that parses `validation table.txt` into an allow/deny map; forbidden validations mark tests as `SKIP`/`EXPECT_DEATH` before execution and fail if violated.
- Fixtures: `Simulation/tests/fixtures/` for deterministic seeds, grid setups, and SID AST snippets; keep sizes small and pinned.
- Determinism guard: shared helper seeds RNGs, freezes FFTW wisdom, and hashes state/output buffers for equality on reruns.
- Metrics: per validation type (e.g., echo peak location, spectrum resonances, attractor residual, rewrite idempotence) with tolerances encoded next to the test.
- Reporting: write JSON per suite to `artifacts/validation/<engine>/<run>.json` (include inputs, metrics, pass/fail) and a rollup summary for CI. Also archive metrics to a root-level `metrics/<engine_family>/` directory (one folder per family) for long-lived evidence.

Engine suites (what to implement)
---------------------------------
- basic_compute_substrate: determinism across repeated runs; stability under epsilon perturbation; scaling microbench over grid sizes; guard that no diffusion/transport semantics are exercised.
- satp_higgs: vacuum stability over timesteps; symmetry breaking trigger and detection; phase transition detection (order parameter crossing); boundedness (energy or amplitude); explicitly avoid mass-conservation/diffusion probes.
- igsoa_gw: signal arrival timing against reference; echo structure shape vs golden CSV; resonance spectrum peaks within tolerance; symmetry response for boundary/geometry flips; forbid diffusion/steady-state tests.
- igsoa_complex: attractor convergence (residual drop over iterations); constraint satisfaction for node coupling; variance suppression vs baseline; ensure no transport/mass-conservation checks.
- sid_ssp: rewrite determinism (same input AST → identical output); invariant preservation across parser → rewrite → mixer; constraint satisfaction on diagrams/packages; no numerical/diffusion probes.
- sid_ternary: I/N/U consistency table coverage; determinism of adjudication; boundary handling for out-of-range inputs; forbid time-evolution/numerical dynamics.

Execution workflow
------------------
- Configure/build: `cmake -S Simulation -B build -DBUILD_TESTS=ON -DBUILD_HARNESS=ON -DUSE_GTEST=ON` then `cmake --build build --target harness_tests`. (If GTest is not installed, CMake will FetchContent-download v1.14.0; requires network on first configure.)
- Run: `ctest -L harness` (per-engine labels `-L gw` etc.); optional `scripts/run_harness.ps1 --engine satp_higgs --report artifacts/validation/`.
- Evidence: JSON outputs live under `artifacts/validation/` with timestamps; metrics are also copied to `metrics/<engine_family>/` (pre-created); human-readable summaries go to `Simulation/tests/validation/summary.md`.

Implementation checklist
------------------------
1) Add gtest/CTest targets and shared `harness_test_util.cpp`.
2) Add policy loader for `validation table.txt` and enforce skip/deny behavior.
3) Create fixtures for each engine with deterministic seeds and small grids/ASTs.
4) Implement metrics per validation type with tolerances; store golden artifacts where needed.
5) Wire reporting to JSON + CI (labels, `ctest` dashboard) and archive metrics to `metrics/<engine_family>/`.
6) Document usage in `Simulation/tests/README.md` and `scripts/run_harness.ps1`, including the metrics archive layout.
