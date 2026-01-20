Harness Review
==============

- `test_cli_harness.py:564-569`: `test_no_infinite_values` initializes `passed = True` and never flips it when `create_engine` fails, so the test can report PASS even if setup errors or the response already contains an error.
- `test_cli_harness.py:551-553`: `test_no_nan_in_outputs` never asserts that `get_state` succeeds; any error response (which contains no floats) will still pass the NaN/Inf check, masking failures to retrieve state.
- `test_cli_harness.py:674-686`: `test_step_advances_state` ignores the results of `set_igsoa_state` and `run_mission`; if either call fails, the comparison can give a false negative/positive without surfacing the failure.
- `test_cli_harness.py:717-719`: `test_metrics_change_after_step` ignores the `run_mission` response, so it may pass even when stepping failed; the metric check can compare stale values.

Suggested fixes
- Fail numeric-invariant tests when setup commands return errors, and validate actual state/metrics values for infinities rather than assuming success.
- Assert success on `set_igsoa_state` and `run_mission` before comparing state or metrics so failures surface directly.
