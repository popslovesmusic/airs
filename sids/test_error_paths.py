"""
Error path tests for SID framework (M16).
Tests edge cases, malformed inputs, and error handling.
"""
from __future__ import annotations

from sid_parser import parse_expression, ParseError
from sid_ast_to_diagram import expr_to_diagram
from sid_crf import attenuate, assign_inu_labels
from sid_validator import validate_package


def test_parser_error_empty_expression():
    """Test parser with empty expression."""
    try:
        parse_expression("")
        assert False, "Should have raised ParseError"
    except ParseError as e:
        assert "Empty" in str(e) or "Unexpected" in str(e)
    print("PASS: test_parser_error_empty_expression")


def test_parser_error_invalid_operator():
    """Test parser with unknown operator."""
    try:
        parse_expression("X(Freedom)")
        assert False, "Should have raised ParseError for unknown operator"
    except ParseError as e:
        assert "Unknown operator" in str(e) or "Unexpected" in str(e)
    print("PASS: test_parser_error_invalid_operator")


def test_parser_error_arity_violation():
    """Test parser arity validation."""
    # C requires 2 arguments
    try:
        parse_expression("C")
        assert False, "Should have raised ParseError for C with 0 args"
    except ParseError as e:
        assert "at least 2 argument" in str(e)
    print("PASS: test_parser_error_arity_violation")


def test_parser_error_unclosed_paren():
    """Test parser with unclosed parenthesis."""
    try:
        parse_expression("P(Freedom")
        assert False, "Should have raised ParseError"
    except ParseError as e:
        assert "end of input" in str(e) or "Expected" in str(e)
    print("PASS: test_parser_error_unclosed_paren")


def test_diagram_error_invalid_structure():
    """Test diagram validation catches invalid structure."""
    from sid_ast_to_diagram import _validate_diagram_structure

    # Edge references non-existent node
    bad_diagram = {
        "id": "d1",
        "nodes": [{"id": "n1", "op": "P"}],
        "edges": [{"id": "e1", "from": "n1", "to": "n999"}]  # n999 doesn't exist
    }

    try:
        _validate_diagram_structure(bad_diagram)
        assert False, "Should have raised ValueError"
    except ValueError as e:
        assert "non-existent" in str(e)
        assert "n999" in str(e)
    print("PASS: test_diagram_error_invalid_structure")


def test_crf_error_missing_constraint_id():
    """Test CRF resolution with missing conflict details."""
    state = {}
    diagram = {}
    conflict_details = {}  # Missing constraint_id

    result = attenuate(conflict_details, state, diagram)

    # Should handle gracefully with default value
    assert result.action == "attenuate"
    assert result.new_state is not None
    # Should use "unknown" as default
    assert "unknown" in str(result.data.get("constraint_id", "unknown"))
    print("PASS: test_crf_error_missing_constraint_id")


def test_inu_labels_empty_constraints():
    """Test I/N/U labeling with no constraints."""
    diagram = {
        "nodes": [{"id": "n1", "op": "P"}],
        "edges": []
    }
    state = {}
    csi = {}
    constraints = []

    labels = assign_inu_labels(diagram, constraints, state, csi)

    # With no constraints, everything should be admissible (I)
    assert labels.get("n1") == "I"
    print("PASS: test_inu_labels_empty_constraints")


def test_validate_package_missing_references():
    """Test package validation catches missing references."""
    package = {
        "dofs": [],
        "csis": [],
        "compartments": [],
        "diagrams": [],
        "states": [
            {
                "id": "s1",
                "diagram_id": "d999",  # Doesn't exist
                "csi_id": "csi999"  # Doesn't exist
            }
        ],
        "constraints": [],
        "rewrite_rules": []
    }

    errors, warnings = validate_package(package)

    # Should have errors about missing references
    assert len(errors) > 0
    error_text = " ".join(errors)
    assert "d999" in error_text or "missing" in error_text.lower()
    print("PASS: test_validate_package_missing_references")


def test_validate_package_duplicate_ids():
    """Test package validation catches duplicate IDs."""
    package = {
        "dofs": [
            {"id": "Freedom"},
            {"id": "Freedom"}  # Duplicate!
        ],
        "csis": [],
        "compartments": [],
        "diagrams": [],
        "states": [],
        "constraints": [],
        "rewrite_rules": []
    }

    errors, warnings = validate_package(package)

    # Should have error about duplicate ID
    assert len(errors) > 0
    error_text = " ".join(errors)
    assert "Duplicate" in error_text or "Freedom" in error_text
    print("PASS: test_validate_package_duplicate_ids")


def test_validate_package_malformed_rewrite():
    """Test validation catches malformed rewrite rules."""
    package = {
        "dofs": [],
        "csis": [],
        "compartments": [],
        "diagrams": [],
        "states": [],
        "constraints": [],
        "rewrite_rules": [
            {
                "id": "r1",
                "pattern_expr": "P(x",  # Invalid syntax!
                "replacement_expr": "O(x)"
            }
        ]
    }

    errors, warnings = validate_package(package)

    # Should have error about invalid expression
    assert len(errors) > 0
    error_text = " ".join(errors)
    assert "invalid" in error_text.lower() or "r1" in error_text
    print("PASS: test_validate_package_malformed_rewrite")


def run_all_tests():
    """Run all error path tests."""
    print("Running error path tests (M16)...\n")

    test_parser_error_empty_expression()
    test_parser_error_invalid_operator()
    test_parser_error_arity_violation()
    test_parser_error_unclosed_paren()
    test_diagram_error_invalid_structure()
    test_crf_error_missing_constraint_id()
    test_inu_labels_empty_constraints()
    test_validate_package_missing_references()
    test_validate_package_duplicate_ids()
    test_validate_package_malformed_rewrite()

    print("\nPASS: All error path tests passed!")


if __name__ == "__main__":
    run_all_tests()
