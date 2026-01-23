Beginner Guide: Building the Simulation Test Harness
====================================================

What this is
------------
We want one reliable test harness for the C++ engines in `Simulation/src/cpp`. It should read the policy in `validation table.txt`, only run allowed checks for each engine family, and save clear evidence to `artifacts/validation/`.

Why it matters
--------------
- Protects against regressions in physics and semantics.
- Keeps runs deterministic so failures are repeatable.
- Produces JSON evidence you can share or audit later.

How we’ll do it (step by step)
------------------------------
1) **Pick the test framework**: use GoogleTest + CTest so tests integrate with the existing CMake build.
2) **Parse the policy**: load `validation table.txt` into a map of allowed/forbidden validation types per engine. If a test would hit a forbidden type, skip or fail fast.
3) **Set up fixtures**: create small, deterministic inputs in `Simulation/tests/fixtures/` (seeds, grids, SID AST snippets). Keep them small so tests stay fast.
4) **Write shared helpers**: a utility to seed RNGs, freeze FFTW wisdom, and hash outputs to check determinism on repeat runs.
5) **Add engine suites**: for each engine family, add tests that match the “Allowed Validations” (e.g., `satp_higgs` gets vacuum stability, symmetry breaking, phase transition detection) and explicitly avoid the forbidden items.
6) **Capture metrics**: define simple metrics per validation (example: echo peak timing for `igsoa_gw`, attractor residuals for `igsoa_complex`, rewrite determinism for `sid_ssp`). Store any golden references alongside the fixtures.
7) **Report results**: after each suite, write JSON to `artifacts/validation/<engine>/<run>.json` plus a short rollup summary for humans; also save a copy of the metrics to `metrics/<engine_family>/` at the repo root (each family gets its own folder).
8) **Wire commands**: add CMake targets like `harness_tests`, labels for `ctest -L harness`, and a helper script `scripts/run_harness.ps1 --engine <name>`.

How to run (once implemented)
-----------------------------
- Configure & build harness: `cmake -S Simulation -B build -DBUILD_TESTS=ON -DBUILD_HARNESS=ON -DUSE_GTEST=ON` then `cmake --build build --target harness_tests` (if GoogleTest isn’t installed, the build will FetchContent-download v1.14.0—requires network once)
- Run all harness tests: `ctest -L harness`
- Run one engine: `ctest -L satp` (or `gw`, `complex`, `ssp`, `ternary`, `basic`)
- Check outputs: see JSON files under `artifacts/validation/`

Where to look next
------------------
- Plan details: `test_harness.md`
- Existing tests you can reuse: `Simulation/tests/`
- Engines and allowed checks: `validation table.txt`
