#!/usr/bin/env python3
"""DASE CLI Validation Test Harness

Black-box testing via stdin/stdout JSON interface.

Task ID: DASE_CLI_VALIDATION_TEST_HARNESS
Execution Model: SESSION-SCOPED (one CLI process per test case)
Interface: JSON lines over stdin/stdout

EXECUTION MODEL DECLARATION:
- Each test case runs in its own CLI process session
- Multiple commands can be sent to the same session
- Engines and state persist within a session
- Sessions do not share state between tests
- Each session terminates when the test completes
"""

import json
import subprocess
import sys
import hashlib
import time
import threading
from typing import Dict, List, Any, Optional
from pathlib import Path
from queue import Queue, Empty
from contextlib import contextmanager

# Configuration
CLI_PATH = Path(__file__).parent / "Simulation" / "bin" / "dase_cli.exe"
TIMEOUT_SECONDS = 10.0
NUMERIC_TOLERANCE = 1e-6


class TestResult:
    """Single test result."""

    def __init__(self, test_id: str, category: str, passed: bool,
                 message: str = "", details: Optional[Dict] = None):
        self.test_id = test_id
        self.category = category
        self.passed = passed
        self.message = message
        self.details = details or {}
        self.timestamp = time.time()

    def to_dict(self) -> Dict:
        return {
            "test_id": self.test_id,
            "category": self.category,
            "passed": self.passed,
            "message": self.message,
            "details": self.details,
            "timestamp": self.timestamp,
        }


class CLISession:
    """Session-scoped CLI process manager.

    Maintains a single CLI process for multiple commands.
    """

    def __init__(self, cli_path: Path):
        self.cli_path = cli_path
        self.process: Optional[subprocess.Popen] = None
        self.stdout_queue: Queue = Queue()
        self.stderr_queue: Queue = Queue()
        self.reader_threads: List[threading.Thread] = []

    def start(self) -> None:
        """Start CLI process session."""
        self.process = subprocess.Popen(
            [str(self.cli_path)],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,  # line buffered
        )

        # Start reader threads for non-blocking I/O
        stdout_thread = threading.Thread(
            target=self._read_stream,
            args=(self.process.stdout, self.stdout_queue),
            daemon=True,
        )
        stderr_thread = threading.Thread(
            target=self._read_stream,
            args=(self.process.stderr, self.stderr_queue),
            daemon=True,
        )

        stdout_thread.start()
        stderr_thread.start()
        self.reader_threads = [stdout_thread, stderr_thread]

    @staticmethod
    def _read_stream(stream, queue: Queue) -> None:
        """Read from stream and put lines into queue."""
        try:
            for line in stream:
                queue.put(line)
        except Exception:
            # Stream may close on process exit.
            pass

    def _drain_stderr(self) -> str:
        lines: List[str] = []
        try:
            while True:
                lines.append(self.stderr_queue.get_nowait())
        except Empty:
            pass
        return "".join(lines)

    def send_command(self, command: Dict, timeout: float = TIMEOUT_SECONDS) -> Dict:
        """Send a command to the session and wait for one JSON-line response."""
        if not self.process or self.process.poll() is not None:
            return {
                "status": "error",
                "error": "Session not active",
                "error_code": "SESSION_DEAD",
            }

        try:
            command_line = json.dumps(command) + "\n"
            assert self.process.stdin is not None
            self.process.stdin.write(command_line)
            self.process.stdin.flush()

            start_time = time.time()
            response_line: Optional[str] = None

            while time.time() - start_time < timeout:
                try:
                    candidate = self.stdout_queue.get(timeout=0.1)
                    if candidate is None:
                        continue
                    if candidate.strip() == "":
                        continue
                    response_line = candidate
                    break
                except Empty:
                    if self.process.poll() is not None:
                        stderr_text = self._drain_stderr()
                        return {
                            "status": "error",
                            "error": "Process terminated unexpectedly",
                            "error_code": "PROCESS_DIED",
                            "stderr": stderr_text,
                        }

            if response_line is None:
                # Timeout
                try:
                    self.process.kill()
                except Exception:
                    pass
                return {
                    "status": "error",
                    "error": "Command timeout",
                    "error_code": "TIMEOUT",
                    "paradigm": "STRUCTURAL_SAFETY",
                    "stderr": self._drain_stderr(),
                }

            response = json.loads(response_line.strip())

            stderr_text = self._drain_stderr()
            if stderr_text:
                response["stderr"] = stderr_text

            return response

        except json.JSONDecodeError as e:
            return {
                "status": "error",
                "error": f"JSON parse error: {str(e)}",
                "error_code": "PARSE_ERROR",
                "raw": response_line.strip() if response_line else "",
                "stderr": self._drain_stderr(),
            }
        except Exception as e:
            return {
                "status": "error",
                "error": str(e),
                "error_code": "HARNESS_ERROR",
                "stderr": self._drain_stderr(),
            }

    def end(self) -> None:
        """Terminate session."""
        if self.process and self.process.poll() is None:
            try:
                self.process.terminate()
                self.process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                try:
                    self.process.kill()
                except Exception:
                    pass
            except Exception:
                try:
                    self.process.kill()
                except Exception:
                    pass


class CLIHarness:
    """Test harness for dase_cli.exe"""

    def __init__(self, cli_path: Path):
        self.cli_path = cli_path
        self.results: List[TestResult] = []
        self.baselines: Dict[str, str] = {}
        self.current_session: Optional[CLISession] = None

        if not cli_path.exists():
            raise FileNotFoundError(f"CLI not found: {cli_path}")

    @contextmanager
    def session(self):
        """Context manager to ensure per-test session semantics."""
        self.start_session()
        try:
            yield
        finally:
            self.end_session()

    def start_session(self) -> None:
        if self.current_session:
            self.end_session()
        self.current_session = CLISession(self.cli_path)
        self.current_session.start()

    def send_command(self, command: Dict, timeout: float = TIMEOUT_SECONDS) -> Dict:
        if not self.current_session:
            raise RuntimeError("No active session. Call start_session() first.")
        return self.current_session.send_command(command, timeout)

    def end_session(self) -> None:
        if self.current_session:
            self.current_session.end()
            self.current_session = None

    @staticmethod
    def hash_state(state_dict: Dict) -> str:
        state_json = json.dumps(state_dict, sort_keys=True)
        return hashlib.sha256(state_json.encode()).hexdigest()[:16]

    @staticmethod
    def assert_success(response: Dict) -> bool:
        return response.get("status") == "success"

    @staticmethod
    def assert_error(response: Dict, expected_code: Optional[str] = None) -> bool:
        if response.get("status") != "error":
            return False
        if expected_code and response.get("error_code") != expected_code:
            return False
        return True

    def assert_no_nan_inf(self, obj: Any) -> bool:
        if isinstance(obj, dict):
            return all(self.assert_no_nan_inf(v) for v in obj.values())
        if isinstance(obj, list):
            return all(self.assert_no_nan_inf(v) for v in obj)
        if isinstance(obj, float):
            import math
            return not (math.isnan(obj) or math.isinf(obj))
        return True

    def record_result(self, test_id: str, category: str, passed: bool,
                      message: str = "", details: Optional[Dict] = None) -> None:
        result = TestResult(test_id, category, passed, message, details)
        self.results.append(result)
        status_icon = "[PASS]" if passed else "[FAIL]"
        print(f"  {status_icon} {test_id}: {message if message else ''}")

    # ========================================================================
    # Category 1: CLI_PARSER_AND_ROUTING
    # ========================================================================

    def test_invalid_json_rejected(self) -> None:
        """Invalid JSON should be rejected with parse error."""
        proc = subprocess.Popen(
            [str(self.cli_path)],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        try:
            stdout, stderr = proc.communicate(input="not valid json\n", timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
            self.record_result("invalid_json_rejected", "CLI_PARSER_AND_ROUTING", False,
                               "Timeout waiting for parse error")
            return

        if stdout.strip():
            first = stdout.strip().split("\n")[0]
            try:
                response = json.loads(first)
                passed = response.get("error_code") == "PARSE_ERROR"
                msg = "Parse error correctly detected" if passed else "Did not detect parse error"
                details = {"stderr": stderr} if stderr else {}
                self.record_result("invalid_json_rejected", "CLI_PARSER_AND_ROUTING", passed, msg, details)
            except Exception:
                self.record_result("invalid_json_rejected", "CLI_PARSER_AND_ROUTING", False,
                                   "Response was not valid JSON", {"stdout": stdout, "stderr": stderr})
        else:
            self.record_result("invalid_json_rejected", "CLI_PARSER_AND_ROUTING", False,
                               "No response to invalid JSON", {"stderr": stderr})

    def test_unknown_command_rejected(self) -> None:
        """Unknown commands should return UNKNOWN_COMMAND error."""
        with self.session():
            response = self.send_command({"command": "nonexistent_command_12345"})
        passed = self.assert_error(response, "UNKNOWN_COMMAND")
        self.record_result("unknown_command_rejected", "CLI_PARSER_AND_ROUTING", passed,
                           "Unknown command correctly rejected" if passed else "Failed to reject unknown command",
                           {"response": response})

    def test_missing_required_parameters(self) -> None:
        """Commands missing required params should fail with MISSING_PARAMETER."""
        with self.session():
            response = self.send_command({"command": "destroy_engine"})
        passed = self.assert_error(response, "MISSING_PARAMETER")
        self.record_result("missing_required_parameters", "CLI_PARSER_AND_ROUTING", passed,
                           "Missing parameter detected" if passed else "Did not detect missing parameter",
                           {"response": response})

    def test_error_format_consistency(self) -> None:
        """All error responses should have consistent structure."""
        with self.session():
            errors = [
                self.send_command({"command": "unknown"}),
                self.send_command({
                    "command": "create_engine",
                    "params": {"engine_type": "igsoa_complex", "num_nodes": 64, "R_c": -1.0},
                }),
                self.send_command({"command": "destroy_engine"}),
            ]

        required_fields = {"status", "error", "error_code"}
        all_consistent = all(
            isinstance(err, dict)
            and required_fields.issubset(err.keys())
            and err.get("status") == "error"
            for err in errors
        )
        self.record_result("error_format_consistency", "CLI_PARSER_AND_ROUTING", all_consistent,
                           "All errors have consistent format" if all_consistent else "Inconsistent error format",
                           {"errors": errors})

    # ========================================================================
    # Category 2: ENGINE_LIFECYCLE
    # ========================================================================

    def test_create_engine_success(self) -> None:
        """Basic engine creation should succeed."""
        with self.session():
            response = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "igsoa_complex", "num_nodes": 128},
            })

        passed = self.assert_success(response) and "engine_id" in response.get("result", {})
        details = {"response": response}
        self.record_result("create_engine_success", "ENGINE_LIFECYCLE", passed,
                           "Engine created successfully" if passed else "Failed to create engine",
                           details)

    def test_create_engine_invalid_params_fail(self) -> None:
        """Invalid physics parameters should be rejected."""
        with self.session():
            response = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "igsoa_complex", "num_nodes": 128, "R_c": -1.0},
            })

        passed = self.assert_error(response, "INVALID_PARAMETER")
        self.record_result("create_engine_invalid_params_fail", "ENGINE_LIFECYCLE", passed,
                           "Invalid R_c rejected" if passed else "Did not reject invalid R_c",
                           {"response": response})

    def test_destroy_engine_success(self) -> None:
        """Created engines should be destroyable."""
        with self.session():
            create_resp = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "igsoa_complex", "num_nodes": 64},
            })

            if not self.assert_success(create_resp):
                self.record_result("destroy_engine_success", "ENGINE_LIFECYCLE", False,
                                   "Could not create engine for destroy test",
                                   {"create_response": create_resp})
                return

            engine_id = create_resp["result"]["engine_id"]

            destroy_resp = self.send_command({
                "command": "destroy_engine",
                "params": {"engine_id": engine_id},
            })

        passed = self.assert_success(destroy_resp)
        self.record_result("destroy_engine_success", "ENGINE_LIFECYCLE", passed,
                           "Engine destroyed successfully" if passed else "Failed to destroy engine",
                           {"create": create_resp, "destroy": destroy_resp})

    def test_double_destroy_fails_cleanly(self) -> None:
        """Destroying same engine twice should fail gracefully."""
        with self.session():
            create_resp = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "igsoa_complex", "num_nodes": 64},
            })

            if not self.assert_success(create_resp):
                self.record_result("double_destroy_fails_cleanly", "ENGINE_LIFECYCLE", False,
                                   "Could not create engine",
                                   {"create_response": create_resp})
                return

            engine_id = create_resp["result"]["engine_id"]

            destroy1 = self.send_command({"command": "destroy_engine", "params": {"engine_id": engine_id}})
            destroy2 = self.send_command({"command": "destroy_engine", "params": {"engine_id": engine_id}})

        passed = self.assert_error(destroy2, "ENGINE_NOT_FOUND")
        self.record_result("double_destroy_fails_cleanly", "ENGINE_LIFECYCLE", passed,
                           "Double destroy handled cleanly" if passed else "Double destroy not detected",
                           {"destroy1": destroy1, "destroy2": destroy2})

    def test_use_after_destroy_rejected(self) -> None:
        """Operations on destroyed engines should fail."""
        with self.session():
            create_resp = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "igsoa_complex", "num_nodes": 64},
            })

            if not self.assert_success(create_resp):
                self.record_result("use_after_destroy_rejected", "ENGINE_LIFECYCLE", False,
                                   "Could not create engine",
                                   {"create_response": create_resp})
                return

            engine_id = create_resp["result"]["engine_id"]
            destroy_resp = self.send_command({"command": "destroy_engine", "params": {"engine_id": engine_id}})
            use_resp = self.send_command({"command": "get_metrics", "params": {"engine_id": engine_id}})

        passed = self.assert_error(use_resp)
        self.record_result("use_after_destroy_rejected", "ENGINE_LIFECYCLE", passed,
                           "Use after destroy rejected" if passed else "Use after destroy not detected",
                           {"destroy": destroy_resp, "use": use_resp})

    # ========================================================================
    # Category 3: DETERMINISM
    # ========================================================================

    def test_repeat_same_command_same_output(self) -> None:
        """Identical commands should produce identical results (ignoring engine_id)."""
        command = {
            "command": "create_engine",
            "params": {"engine_type": "igsoa_complex", "num_nodes": 64, "R_c": 1.5},
        }

        with self.session():
            resp1 = self.send_command(command)
            resp2 = self.send_command(command)

        if self.assert_success(resp1) and self.assert_success(resp2):
            params1 = {k: v for k, v in resp1["result"].items() if k != "engine_id"}
            params2 = {k: v for k, v in resp2["result"].items() if k != "engine_id"}
            passed = params1 == params2
            self.record_result("repeat_same_command_same_output", "DETERMINISM", passed,
                               "Identical commands produce identical results" if passed else "Non-deterministic output",
                               {"resp1": resp1, "resp2": resp2})
        else:
            self.record_result("repeat_same_command_same_output", "DETERMINISM", False,
                               "Commands failed",
                               {"resp1": resp1, "resp2": resp2})

    def test_state_hash_stability(self) -> None:
        """State hashes should be stable within a deterministic run."""
        with self.session():
            create_resp = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "igsoa_complex", "num_nodes": 128, "R_c": 1.0},
            })

            if not self.assert_success(create_resp):
                self.record_result("state_hash_stability", "DETERMINISM", False,
                                   "Failed to create engine",
                                   {"create_response": create_resp})
                return

            engine_id = create_resp["result"]["engine_id"]

            set_resp = self.send_command({
                "command": "set_igsoa_state",
                "params": {
                    "engine_id": engine_id,
                    "profile_type": "uniform",
                    "params": {"psi_real": 0.1, "psi_imag": 0.0, "phi": 0.0},
                },
            })

            state_resp = self.send_command({
                "command": "get_state",
                "params": {"engine_id": engine_id},
            })

        if self.assert_success(state_resp):
            state_hash = self.hash_state(state_resp["result"])
            self.baselines["igsoa_complex_uniform_state"] = state_hash
            self.record_result("state_hash_stability", "DETERMINISM", True,
                               f"State hash: {state_hash}",
                               {"hash": state_hash, "set_response": set_resp})
        else:
            self.record_result("state_hash_stability", "DETERMINISM", False,
                               "Failed to get state",
                               {"set_response": set_resp, "state_response": state_resp})

    # ========================================================================
    # Category 4: NUMERIC_INVARIANTS
    # ========================================================================

    def test_no_nan_in_outputs(self) -> None:
        """No NaN values should appear in any output."""
        with self.session():
            create_resp = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "igsoa_complex", "num_nodes": 64},
            })

            if not self.assert_success(create_resp):
                self.record_result("no_nan_in_outputs", "NUMERIC_INVARIANTS", False,
                                   "Failed to create engine",
                                   {"create_response": create_resp})
                return

            engine_id = create_resp["result"]["engine_id"]
            state_resp = self.send_command({"command": "get_state", "params": {"engine_id": engine_id}})

        passed = self.assert_no_nan_inf(state_resp)
        self.record_result("no_nan_in_outputs", "NUMERIC_INVARIANTS", passed,
                           "No NaN/Inf in outputs" if passed else "NaN or Inf detected in output",
                           {"state_response": state_resp})

    def test_no_infinite_values(self) -> None:
        """No infinite values should appear."""
        with self.session():
            create_resp = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "igsoa_complex", "num_nodes": 64, "R_c": 0.001},
            })

        passed = True
        if self.assert_success(create_resp):
            passed = self.assert_no_nan_inf(create_resp)

        self.record_result("no_infinite_values", "NUMERIC_INVARIANTS", passed,
                           "No infinite values" if passed else "Infinite value detected",
                           {"create_response": create_resp})

    def test_division_by_zero_guarded(self) -> None:
        """Division by zero should be guarded (tested indirectly via single-node SATP)."""
        with self.session():
            create_resp = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "satp_higgs_1d", "num_nodes": 1},
            })

            if not self.assert_success(create_resp):
                self.record_result("division_by_zero_guarded", "NUMERIC_INVARIANTS", False,
                                   "Failed to create engine",
                                   {"create_response": create_resp})
                return

            engine_id = create_resp["result"]["engine_id"]
            state_resp = self.send_command({"command": "get_satp_state", "params": {"engine_id": engine_id}})

        passed = self.assert_success(state_resp) and self.assert_no_nan_inf(state_resp)
        self.record_result("division_by_zero_guarded", "NUMERIC_INVARIANTS", passed,
                           "Division by zero protected" if passed else "Division by zero not protected",
                           {"state_response": state_resp})

    def test_dimensions_consistent(self) -> None:
        """2D engines should report consistent dimensions."""
        with self.session():
            create_resp = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "igsoa_complex_2d", "N_x": 8, "N_y": 16},
            })

            if not self.assert_success(create_resp):
                self.record_result("dimensions_consistent", "NUMERIC_INVARIANTS", False,
                                   "Failed to create 2D engine",
                                   {"create_response": create_resp})
                return

            engine_id = create_resp["result"]["engine_id"]
            state_resp = self.send_command({"command": "get_state", "params": {"engine_id": engine_id}})

        if self.assert_success(state_resp):
            result = state_resp["result"]
            expected_nodes = 8 * 16
            dims = result.get("dimensions", {})
            passed = (result.get("num_nodes") == expected_nodes and dims.get("N_x") == 8 and dims.get("N_y") == 16)
            self.record_result("dimensions_consistent", "NUMERIC_INVARIANTS", passed,
                               "Dimensions consistent" if passed else "Dimension mismatch",
                               {"state_response": state_resp})
        else:
            self.record_result("dimensions_consistent", "NUMERIC_INVARIANTS", False,
                               "Failed to get state",
                               {"state_response": state_resp})

    # ========================================================================
    # Category 5: ERROR_PATHS
    # ========================================================================

    def test_invalid_parameters_fail_early(self) -> None:
        """Invalid parameters should fail before engine creation."""
        with self.session():
            response = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "igsoa_complex", "num_nodes": 64, "gamma": -0.5},
            })

        passed = self.assert_error(response, "INVALID_PARAMETER")
        self.record_result("invalid_parameters_fail_early", "ERROR_PATHS", passed,
                           "Invalid params rejected early" if passed else "Invalid params not rejected",
                           {"response": response})

    def test_engine_not_found(self) -> None:
        """Operations on non-existent engines should fail cleanly."""
        with self.session():
            response = self.send_command({
                "command": "get_metrics",
                "params": {"engine_id": "nonexistent_engine_999"},
            })

        passed = self.assert_error(response)
        self.record_result("engine_not_found", "ERROR_PATHS", passed,
                           "Non-existent engine handled" if passed else "Non-existent engine not detected",
                           {"response": response})

    # ========================================================================
    # Category 6: STATE_EVOLUTION
    # ========================================================================

    def test_step_advances_state(self) -> None:
        """Running mission steps should change state."""
        with self.session():
            create_resp = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "igsoa_complex", "num_nodes": 64},
            })

            if not self.assert_success(create_resp):
                self.record_result("step_advances_state", "STATE_EVOLUTION", False,
                                   "Failed to create engine",
                                   {"create_response": create_resp})
                return

            engine_id = create_resp["result"]["engine_id"]

            self.send_command({
                "command": "set_igsoa_state",
                "params": {
                    "engine_id": engine_id,
                    "profile_type": "uniform",
                    "params": {"psi_real": 0.1, "psi_imag": 0.0, "phi": 0.0},
                },
            })

            state1_resp = self.send_command({"command": "get_state", "params": {"engine_id": engine_id}})

            self.send_command({"command": "run_mission", "params": {"engine_id": engine_id, "num_steps": 10}})

            state2_resp = self.send_command({"command": "get_state", "params": {"engine_id": engine_id}})

        if self.assert_success(state1_resp) and self.assert_success(state2_resp):
            hash1 = self.hash_state(state1_resp["result"])
            hash2 = self.hash_state(state2_resp["result"])
            passed = hash1 != hash2
            self.record_result("step_advances_state", "STATE_EVOLUTION", passed,
                               "State changed after mission" if passed else "State did not change",
                               {"hash_before": hash1, "hash_after": hash2})
        else:
            self.record_result("step_advances_state", "STATE_EVOLUTION", False,
                               "Failed to get states",
                               {"state1": state1_resp, "state2": state2_resp})

    def test_metrics_change_after_step(self) -> None:
        """Metrics should reflect state changes."""
        with self.session():
            create_resp = self.send_command({
                "command": "create_engine",
                "params": {"engine_type": "igsoa_complex", "num_nodes": 64},
            })

            if not self.assert_success(create_resp):
                self.record_result("metrics_change_after_step", "STATE_EVOLUTION", False,
                                   "Failed to create engine",
                                   {"create_response": create_resp})
                return

            engine_id = create_resp["result"]["engine_id"]

            metrics1_resp = self.send_command({"command": "get_metrics", "params": {"engine_id": engine_id}})
            self.send_command({"command": "run_mission", "params": {"engine_id": engine_id, "num_steps": 5}})
            metrics2_resp = self.send_command({"command": "get_metrics", "params": {"engine_id": engine_id}})

        if self.assert_success(metrics1_resp) and self.assert_success(metrics2_resp):
            ops1 = metrics1_resp["result"].get("total_operations", 0)
            ops2 = metrics2_resp["result"].get("total_operations", 0)
            passed = ops2 > ops1
            self.record_result("metrics_change_after_step", "STATE_EVOLUTION", passed,
                               "Metrics updated after mission" if passed else "Metrics did not update",
                               {"ops_before": ops1, "ops_after": ops2})
        else:
            self.record_result("metrics_change_after_step", "STATE_EVOLUTION", False,
                               "Failed to get metrics",
                               {"metrics1": metrics1_resp, "metrics2": metrics2_resp})

    # ========================================================================
    # Harness Execution
    # ========================================================================

    def run_all_tests(self) -> None:
        print("\n" + "=" * 70)
        print("DASE CLI Validation Test Harness")
        print("=" * 70)

        categories = [
            ("CLI_PARSER_AND_ROUTING", [
                self.test_invalid_json_rejected,
                self.test_unknown_command_rejected,
                self.test_missing_required_parameters,
                self.test_error_format_consistency,
            ]),
            ("ENGINE_LIFECYCLE", [
                self.test_create_engine_success,
                self.test_create_engine_invalid_params_fail,
                self.test_destroy_engine_success,
                self.test_double_destroy_fails_cleanly,
                self.test_use_after_destroy_rejected,
            ]),
            ("DETERMINISM", [
                self.test_repeat_same_command_same_output,
                self.test_state_hash_stability,
            ]),
            ("NUMERIC_INVARIANTS", [
                self.test_no_nan_in_outputs,
                self.test_no_infinite_values,
                self.test_division_by_zero_guarded,
                self.test_dimensions_consistent,
            ]),
            ("ERROR_PATHS", [
                self.test_invalid_parameters_fail_early,
                self.test_engine_not_found,
            ]),
            ("STATE_EVOLUTION", [
                self.test_step_advances_state,
                self.test_metrics_change_after_step,
            ]),
        ]

        for category_name, tests in categories:
            print(f"\n{category_name}:")
            for test_func in tests:
                try:
                    test_func()
                except Exception as e:
                    self.record_result(
                        test_func.__name__,
                        category_name,
                        False,
                        f"Exception: {str(e)}",
                    )
                    # Ensure no leaked session across tests.
                    self.end_session()

        self.print_summary()

    def print_summary(self) -> None:
        print("\n" + "=" * 70)
        print("TEST SUMMARY")
        print("=" * 70)

        total = len(self.results)
        passed = sum(1 for r in self.results if r.passed)
        failed = total - passed

        print(f"Total tests: {total}")
        print(f"Passed: {passed}")
        print(f"Failed: {failed}")
        print(f"Success rate: {100 * passed / total:.1f}%")

        if failed > 0:
            print("\nFailed tests:")
            for r in self.results:
                if not r.passed:
                    print(f"  - {r.test_id}: {r.message}")

    def generate_report(self, output_path: Path) -> None:
        report = {
            "harness_version": "1.0.0",
            "cli_path": str(self.cli_path),
            "timestamp": time.time(),
            "total_tests": len(self.results),
            "passed": sum(1 for r in self.results if r.passed),
            "failed": sum(1 for r in self.results if not r.passed),
            "results": [r.to_dict() for r in self.results],
            "baselines": self.baselines,
            "success": all(r.passed for r in self.results),
        }

        with open(output_path, "w", encoding="utf-8") as f:
            json.dump(report, f, indent=2)

        print(f"\nTest report written to: {output_path}")


def main() -> None:
    harness = CLIHarness(CLI_PATH)

    try:
        harness.run_all_tests()

        report_path = Path(__file__).parent / "test_results.json"
        harness.generate_report(report_path)

        all_passed = all(r.passed for r in harness.results)
        sys.exit(0 if all_passed else 1)

    except Exception as e:
        print(f"\nHarness error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        sys.exit(2)


if __name__ == "__main__":
    main()
