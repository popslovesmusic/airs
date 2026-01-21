# SIM_FIX_GW_SID_REPORT

Task: SIM_FIX_GW_TESTS_SID_INVARIANTS_002
Date: 2026-01-19
Scope root: Simulation/

## Summary
Completed GW engine TODOs and added deterministic validation coverage, then converted SID invariant enforcement from assert-only checks to runtime exceptions for Release builds. Updated SID tests to current APIs and added negative invariant tests.

## GW Engine Updates
- Fractional solver: kernel cache interpolation, exact Caputo evaluation, deterministic kernel validation, and asymptotic expansion implemented. Input size validation added.
- Symmetry field: full-grid gradient computation with explicit boundary handling, potential/derivative implementations confirmed, and CSV export now emits data.
- Projection operators: TT-projected strain extraction used; gauge transform retains derived amplitude/phase.

Files:
- Simulation/src/cpp/igsoa_gw_engine/core/fractional_solver.cpp
- Simulation/src/cpp/igsoa_gw_engine/core/symmetry_field.cpp
- Simulation/src/cpp/igsoa_gw_engine/core/projection_operators.cpp

## SID Invariant Enforcement
- Replaced assert-based checks with runtime exceptions for role locking, mask validity, and parameter constraints.
- Added conservation and mixer boundedness runtime violations.

Files:
- Simulation/src/cpp/sid_ssp/sid_semantic_processor.hpp
- Simulation/src/cpp/sid_ssp/sid_mixer.hpp

## Test Updates and Additions
- Updated core SID C++ tests to current APIs (parser implementation, diagram builder, validator behavior, mixer usage).
- Added SID invariant negative tests covering role, mask validity, conservation, and boundedness.
- Added GW fractional kernel validation and boundary gradient cache tests.

Files:
- Simulation/tests/test_sid_core.cpp
- Simulation/tests/test_sid_invariants.cpp
- Simulation/tests/test_gw_engine_basic.cpp

## Behavior Notes
- Runtime invariants now throw std::logic_error or std::runtime_error in Release as required.
- Mixer boundedness now throws when scale exceeds cap instead of clamping.
- Conservation check throws when error exceeds configured tolerance, with before/after totals.

## Tests Not Run
- C++ GW tests (test_gw_engine_basic.cpp)
- C++ SID tests (test_sid_core.cpp, test_sid_invariants.cpp)

## Suggested Commands
- Build and run GW test binary (project-specific build steps).
- Build and run SID test binaries (project-specific build steps).
