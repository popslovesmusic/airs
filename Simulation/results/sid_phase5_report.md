SID Phase 5 Report

Date: 2026-01-18
Scope: SID tests, benchmarks, and engine availability

Build
- cmake --build build --config Release --target test_sid_core test_sid_rewrite test_sid_integration test_sid_regression test_sid_capi_json benchmark_sid

Tests
- test_sid_core: 24 passed, 0 failed
- test_sid_rewrite: 3 passed, 0 failed
- test_sid_integration: 3 passed, 0 failed
- test_sid_regression: 12 passed, 0 failed
- test_sid_capi_json: 1 passed, 0 failed
- test_sid_ports: 16 passed, 0 failed

Benchmark
- benchmark_sid:
  Parse iterations: 10000 in 27 ms
  Cycle detection: false in 3 us
  Cycle detection (10k): false in 4 ms
  Rewrite applied: true in 30 us

Engine availability (get_capabilities)
- phase4b
- igsoa_complex
- igsoa_complex_2d
- igsoa_complex_3d
- satp_higgs_1d
- satp_higgs_2d
- satp_higgs_3d
- sid_ternary
- igsoa_gw
CPU features: avx2=true, fma=true, avx512=false

Side-by-side validation
- tests/test_sid_validation.py: PASS (expanded expressions)
