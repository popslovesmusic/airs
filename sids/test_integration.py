"""
Integration tests for the SID framework.
Tests the complete pipeline: parse → diagram → validate → rewrite → stability
"""
from __future__ import annotations

from sid_parser import parse_expression
from sid_ast_to_diagram import expr_to_diagram
from sid_validator import validate_package
from sid_rewrite import apply_rewrites_to_package
from sid_stability import is_structurally_stable, compute_stability_metrics
from sid_crf import assign_inu_labels


def test_end_to_end_simple_expression():
    """Test parsing a simple expression and converting to diagram."""
    # Parse expression
    expr = parse_expression("P(Freedom)")

    # Convert to diagram
    diagram = expr_to_diagram(expr, "d_test")

    # Verify diagram structure
    assert "nodes" in diagram
    assert "edges" in diagram
    assert len(diagram["nodes"]) >= 1

    # Check that P node exists with Freedom DOF
    found = False
    for node in diagram["nodes"]:
        if node.get("op") == "P" and "Freedom" in node.get("dof_refs", []):
            found = True
    assert found, "P(Freedom) node not found in diagram"

    print("PASS: test_end_to_end_simple_expression")


def test_end_to_end_operator_arity():
    """Test that operator arity validation works."""
    from sid_parser import ParseError

    # Should accept C(a, b) with 2 arguments
    try:
        expr = parse_expression("C(P(Freedom), P(Justice))")
        assert True
    except ParseError:
        assert False, "C with 2 arguments should be valid"

    # Should reject C with 0 arguments (bare operator)
    try:
        expr = parse_expression("C")
        assert False, "C with 0 arguments should have failed arity validation"
    except ParseError as e:
        assert "at least 2 argument" in str(e)

    # Should reject C(a) with 1 argument
    try:
        expr = parse_expression("C(P(Freedom))")
        assert False, "C(a) should have failed arity validation"
    except ParseError as e:
        assert "at least 2 argument" in str(e)

    print("PASS: test_end_to_end_operator_arity")


def test_end_to_end_package_validation():
    """Test package validation with a complete SID package."""
    package = {
        "dofs": [
            {"id": "Freedom", "type": "continuous"}
        ],
        "csis": [
            {
                "id": "csi1",
                "allowed_dofs": ["Freedom"],
                "allowed_pairs": []
            }
        ],
        "compartments": [
            {"id": "c1", "name": "Main"}
        ],
        "diagrams": [
            {
                "id": "d1",
                "nodes": [
                    {"id": "n1", "op": "P", "dof_refs": ["Freedom"]}
                ],
                "edges": []
            }
        ],
        "states": [
            {
                "id": "s1",
                "diagram_id": "d1",
                "csi_id": "csi1",
                "inu_labels": {}
            }
        ],
        "constraints": [],
        "rewrite_rules": []
    }

    errors, warnings = validate_package(package)

    # Should have no errors for valid package
    assert len(errors) == 0, f"Unexpected errors: {errors}"

    print("PASS: test_end_to_end_package_validation")


def test_end_to_end_inu_labeling():
    """Test I/N/U label assignment."""
    diagram = {
        "nodes": [
            {"id": "n1", "op": "P", "dof_refs": ["Freedom"]},
            {"id": "n2", "op": "O", "irreversible": True}
        ],
        "edges": [
            {"id": "e1", "from": "n1", "to": "n2", "label": "arg"}
        ]
    }

    state = {}
    csi = {"allowed_dofs": ["Freedom"], "allowed_pairs": []}
    constraints = []

    # Assign labels
    labels = assign_inu_labels(diagram, constraints, state, csi)

    # With no constraints, all should be "I" (admissible)
    assert labels.get("n1") == "I", f"Expected I, got {labels.get('n1')}"
    assert labels.get("n2") == "I", f"Expected I, got {labels.get('n2')}"
    assert labels.get("e1") == "I", f"Expected I, got {labels.get('e1')}"

    print("PASS: test_end_to_end_inu_labeling")


def test_end_to_end_stability_check():
    """Test structural stability checking."""
    package = {
        "states": [
            {
                "id": "s1",
                "diagram_id": "d1",
                "csi_id": "csi1",
                "inu_labels": {"n1": "I"},
                "loop_history": [
                    {"inu_labels": {"n1": "I"}},
                    {"inu_labels": {"n1": "I"}}
                ]
            }
        ],
        "diagrams": [
            {
                "id": "d1",
                "nodes": [{"id": "n1", "op": "P", "dof_refs": ["Freedom"]}],
                "edges": []
            }
        ],
        "csis": [
            {
                "id": "csi1",
                "allowed_dofs": ["Freedom"],
                "allowed_pairs": []
            }
        ],
        "constraints": [],
        "rewrite_rules": []
    }

    is_stable, satisfied, message = is_structurally_stable(package, "s1", "d1")

    # Should be stable (no rewrites, loop converged)
    assert is_stable is True, f"Expected stable, got: {message}"
    assert len(satisfied) > 0, "Expected some conditions satisfied"

    print("PASS: test_end_to_end_stability_check")


def test_end_to_end_metrics_computation():
    """Test metrics computation after stability."""
    package = {
        "states": [
            {
                "id": "s1",
                "inu_labels": {"n1": "I", "n2": "I", "n3": "U"},
                "loop_history": [
                    {"inu_labels": {"n1": "I", "n2": "I", "n3": "U"}},
                    {"inu_labels": {"n1": "I", "n2": "I", "n3": "I"}}
                ]
            }
        ],
        "diagrams": [
            {
                "id": "d1",
                "nodes": [
                    {"id": "n1", "op": "P"},
                    {"id": "n2", "op": "O"},
                    {"id": "n3", "op": "C"}
                ],
                "edges": []
            }
        ]
    }

    metrics = compute_stability_metrics(package, "s1", "d1")

    # Check that metrics exist
    assert "admissible_volume" in metrics
    assert "collapse_ratio" in metrics
    assert "gradient_coherence" in metrics
    assert "loop_gain" in metrics

    # Check some specific values
    assert metrics["collapse_count"] == 1
    assert metrics["coupling_count"] == 1

    print("PASS: test_end_to_end_metrics_computation")


def test_end_to_end_edge_label_consistency():
    """Test that edge labels are now consistent (all 'arg')."""
    expr = parse_expression("O(P(Freedom))")
    diagram = expr_to_diagram(expr, "d_test")

    # All edges should have label 'arg'
    for edge in diagram.get("edges", []):
        label = edge.get("label")
        assert label == "arg", f"Expected 'arg', got {label!r}"

    print("PASS: test_end_to_end_edge_label_consistency")


def test_end_to_end_crf_pure_functions():
    """Test that CRF resolution functions don't mutate state."""
    from sid_crf import attenuate, defer, partition

    original_state = {"existing_key": "value"}
    conflict_details = {"constraint_id": "c1"}
    diagram = {}

    # Call attenuate
    result = attenuate(conflict_details, original_state, diagram)

    # Original state should be unchanged
    assert "attenuated_constraints" not in original_state, "Original state was mutated!"
    assert original_state == {"existing_key": "value"}

    # New state should have the changes
    assert result.new_state is not None
    assert "attenuated_constraints" in result.new_state
    assert "c1" in result.new_state["attenuated_constraints"]

    # Test defer
    original_state2 = {}
    result2 = defer({"type": "test"}, original_state2, diagram)
    assert "deferred_conflicts" not in original_state2
    assert "deferred_conflicts" in result2.new_state

    # Test partition
    original_state3 = {}
    result3 = partition({"conflicting_elements": ["e1"]}, original_state3, diagram)
    assert "partitioned_elements" not in original_state3
    assert "partitioned_elements" in result3.new_state

    print("PASS: test_end_to_end_crf_pure_functions")


def test_end_to_end_rewrite_result_immutable():
    """Test that RewriteResult is immutable."""
    from sid_rewrite import RewriteResult
    import copy

    original_diagram = {"nodes": [{"id": "n1"}], "edges": []}
    result = RewriteResult(
        applied=True,
        diagram=copy.deepcopy(original_diagram),
        messages=("Test message",)
    )

    # Verify frozen dataclass
    try:
        result.applied = False
        assert False, "RewriteResult should be frozen"
    except AttributeError:
        pass  # Expected

    # Verify messages is tuple (immutable)
    assert isinstance(result.messages, tuple)

    print("PASS: test_end_to_end_rewrite_result_immutable")


def run_all_tests():
    """Run all integration tests."""
    print("Running integration tests...\n")

    test_end_to_end_simple_expression()
    test_end_to_end_operator_arity()
    test_end_to_end_package_validation()
    test_end_to_end_inu_labeling()
    test_end_to_end_stability_check()
    test_end_to_end_metrics_computation()
    test_end_to_end_edge_label_consistency()
    test_end_to_end_crf_pure_functions()
    test_end_to_end_rewrite_result_immutable()

    print("\nPASS: All integration tests passed!")


if __name__ == "__main__":
    run_all_tests()
