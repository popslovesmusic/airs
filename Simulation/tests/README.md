# Simulation Tests

## Harness quickstart
- Configure (if not already):
  cmake -S Simulation -B build -DBUILD_TESTS=ON -DBUILD_HARNESS=ON -DUSE_GTEST=ON -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=D:/airs/build/_deps/googletest-src/googletest-1.14.0 -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
- Build: cmake --build build --target harness_tests --config Debug
- Run all harness tests (VS generator): cd build && ctest -C Debug -L harness
- Run one family: cd build && ctest -C Debug -L gw (labels: gw, satp, complex, ssp, ternary, basic)
- Wrapper script: powershell -File scripts/run_harness.ps1 -Config Debug -Engine gw

## Fixtures
Stored in Simulation/tests/fixtures/:
- basic_kernel.txt: baseline kernel values
- satp_state.txt: phi evolution for vacuum stability trend
- igsoa_gw_echo.csv: echo waveform sample
- igsoa_complex_state.txt: residual/variance per iteration
- sid_ssp_ast.txt: sample AST for rewrite determinism
- sid_ternary_cases.txt: adjudication truth table samples

## Outputs
- Metrics: metrics/<engine_family>/...json
- Validation artifacts (future): rtifacts/validation/<engine>/

## TODO
- Replace placeholder skips with real engine invocations and golden comparisons.
- Add golden outputs and tighter tolerances per validation type.
- Expand reporting rollup to Simulation/tests/validation/summary.md.
