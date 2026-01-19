#!/usr/bin/env python3
"""
Test runner for all SID tests.
"""
from __future__ import annotations

import sys


def run_all_tests() -> int:
    """Run all test suites and return exit code."""
    print("=" * 60)
    print("SID Framework Test Suite")
    print("=" * 60)
    print()

    failures = 0

    # Run CRF tests
    try:
        import test_sid_crf
        test_sid_crf.run_all_tests()
        print()
    except Exception as e:
        print(f"FAIL: CRF tests failed: {e}")
        failures += 1

    # Run stability tests
    try:
        import test_sid_stability
        test_sid_stability.run_all_tests()
        print()
    except Exception as e:
        print(f"FAIL: Stability tests failed: {e}")
        failures += 1

    # Summary
    print("=" * 60)
    if failures == 0:
        print("PASS: ALL TEST SUITES PASSED")
        print("=" * 60)
        return 0
    else:
        print(f"FAIL: {failures} TEST SUITE(S) FAILED")
        print("=" * 60)
        return 1


if __name__ == "__main__":
    sys.exit(run_all_tests())
