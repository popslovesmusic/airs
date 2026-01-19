"""
Parametrized tests for SID framework (Minor Issue #17).

Uses pytest.mark.parametrize to reduce code duplication and increase coverage.
"""
from __future__ import annotations

import pytest

from sid_parser import parse_expression, ParseError, tokenize
from sid_crf import resolve_conflict, ConflictResolution
from sid_stability import (
    check_no_admissible_rewrites,
    check_only_identity_rewrites,
    check_loop_convergence,
)


# Parser parametrized tests
@pytest.mark.parametrize("expr,expected_type", [
    ("P(Freedom)", "op"),
    ("Freedom", "atom"),
    ("S+(A, B)", "op"),
    ("O(P(X))", "op"),
])
def test_parse_expression_types(expr, expected_type):
    """Test parsing various expression types."""
    result = parse_expression(expr)
    assert hasattr(result, "__class__")
    if expected_type == "atom":
        assert result.__class__.__name__ == "Atom"
    else:
        assert result.__class__.__name__ == "OpExpr"


@pytest.mark.parametrize("operator,min_args,max_args", [
    ("P", 1, 1),
    ("S+", 1, None),
    ("S-", 1, None),
    ("O", 1, 1),
    ("C", 2, 2),
    ("T", 1, 1),
])
def test_operator_arity_constraints(operator, min_args, max_args):
    """Test that operator arity constraints are enforced."""
    # Valid: minimum args (use bare atoms which are valid)
    if min_args == 1:
        valid_expr = f"{operator}(Freedom)"
        result = parse_expression(valid_expr)
        assert result is not None

    # Valid: two args for operators that allow it
    if max_args is None or max_args >= 2:
        if operator in ("S+", "S-", "C"):
            valid_expr = f"{operator}(Freedom, Energy)"
            result = parse_expression(valid_expr)
            assert result is not None

    # Invalid: zero args when minimum is 1
    if min_args >= 1:
        invalid_expr = f"{operator}()"
        with pytest.raises(ParseError):  # "Unexpected token rparen" before arity check
            parse_expression(invalid_expr)

    # Invalid: too many args when max is defined
    if max_args == 1:
        invalid_expr = f"{operator}(Freedom, Energy)"
        with pytest.raises(ParseError, match="accepts at most"):
            parse_expression(invalid_expr)


@pytest.mark.parametrize("invalid_expr,expected_error", [
    ("", "Empty expression"),
    ("P(", "Unexpected end"),
    ("P)", "Unexpected token"),
    ("@invalid", "Unexpected character"),
    ("INVALID(X)", "Unknown operator"),  # INVALID is not a valid operator
])
def test_parse_errors(invalid_expr, expected_error):
    """Test that invalid expressions raise appropriate errors."""
    with pytest.raises((ParseError, ValueError)):
        result = parse_expression(invalid_expr)
        # If no error, fail the test
        pytest.fail(f"Expected error for {invalid_expr!r}, got {result}")


# CRF parametrized tests
@pytest.mark.parametrize("conflict_type,expected_action", [
    ("soft_violation", "attenuate"),
    ("temporal_mismatch", "defer"),
    ("dof_interference", "partition"),
    ("scope_overflow", "escalate"),
    ("ambiguous_choice", "bifurcate"),
    ("hard_violation", "halt"),
])
def test_conflict_resolution_routing(conflict_type, expected_action):
    """Test that conflict types route to correct resolution procedures."""
    conflict_details = {"type": conflict_type, "reason": "test"}
    state = {"id": "s1"}
    diagram = {"id": "d1", "nodes": [], "edges": []}

    result = resolve_conflict(conflict_type, conflict_details, state, diagram)

    assert isinstance(result, ConflictResolution)
    assert result.action == expected_action
    assert result.new_state is not None


@pytest.mark.parametrize("labels,expected_admissible", [
    ({"n1": "I", "n2": "I"}, (True, "All elements admissible")),
    ({"n1": "I", "n2": "U"}, (True, "Admissible with 1 unresolved")),
    ({"n1": "N", "n2": "I"}, (False, "Element n1 is N")),
    ({"n1": "U", "n2": "U"}, (True, "Admissible with 2 unresolved")),
])
def test_check_admissible_various_labels(labels, expected_admissible):
    """Test admissibility checking with various I/N/U label combinations."""
    from sid_crf import check_admissible

    state = {"inu_labels": labels}
    is_admissible, message = check_admissible(state)

    expected_result, expected_msg_fragment = expected_admissible
    assert is_admissible == expected_result
    assert expected_msg_fragment in message


# Stability parametrized tests
@pytest.mark.parametrize("pattern,replacement,is_identity", [
    ("P(X)", "P(X)", True),
    ("S+(A)", "S+(A)", True),
    ("P(X)", "S+(X)", False),
    ("O(P(X))", "P(X)", False),
])
def test_identity_rewrite_detection(pattern, replacement, is_identity):
    """Test detection of identity rewrites."""
    rewrite_rules = [{
        "id": "r1",
        "pattern_expr": pattern,
        "replacement_expr": replacement,
    }]
    diagram = {"id": "d1"}

    result, msg = check_only_identity_rewrites(rewrite_rules, diagram)

    assert result == is_identity


@pytest.mark.parametrize("loop_history,should_converge", [
    (
        [
            {"inu_labels": {"n1": "I", "n2": "I"}},
            {"inu_labels": {"n1": "I", "n2": "I"}},
        ],
        True,
    ),
    (
        [
            {"inu_labels": {"n1": "I", "n2": "I"}},
            {"inu_labels": {"n1": "U", "n2": "I"}},
        ],
        False,
    ),
    (
        [{"inu_labels": {"n1": "I"}}],
        False,  # Insufficient history
    ),
])
def test_loop_convergence_various_histories(loop_history, should_converge):
    """Test loop convergence with various history patterns."""
    state = {"loop_history": loop_history}

    result, msg = check_loop_convergence(state, tolerance=0.3)

    assert result == should_converge


# Tokenizer parametrized tests
@pytest.mark.parametrize("text,expected_tokens", [
    ("P", [("op", "P")]),
    ("Freedom", [("ident", "Freedom")]),
    ("(", [("lparen", "(")]),
    (")", [("rparen", ")")]),
    (",", [("comma", ",")]),
    ("S+", [("op", "S+")]),
    ("S-", [("op", "S-")]),
])
def test_tokenize_individual_tokens(text, expected_tokens):
    """Test tokenization of individual tokens."""
    tokens = tokenize(text)

    assert len(tokens) == len(expected_tokens)
    for token, (expected_kind, expected_value) in zip(tokens, expected_tokens):
        assert token.kind == expected_kind
        assert token.value == expected_value


@pytest.mark.parametrize("text,expected_count", [
    ("P(Freedom)", 4),  # P, (, Freedom, )
    ("S+(A, B)", 6),     # S+, (, A, ,, B, )
    ("O(P(X))", 7),      # O, (, P, (, X, ), ) - 7 tokens
])
def test_tokenize_expression_token_count(text, expected_count):
    """Test that complex expressions produce expected token count."""
    tokens = tokenize(text)
    assert len(tokens) == expected_count


# Edge cases and boundary conditions
@pytest.mark.parametrize("operator,num_args", [
    ("S+", 1),
    ("S+", 2),
    ("S+", 3),
    ("S+", 5),
])
def test_superposition_variable_arity(operator, num_args):
    """Test that S+ and S- accept variable number of arguments."""
    # Use bare atoms with names that aren't operators (avoid C, O, P, T, S+, S-)
    args_list = [f"X{i}" for i in range(num_args)]  # X0, X1, X2, ...
    args_str = ", ".join(args_list)
    expr = f"{operator}({args_str})"
    result = parse_expression(expr)
    assert result is not None
    assert result.op == operator
    assert len(result.args) == num_args


@pytest.mark.parametrize("prefix,valid", [
    ("", True),   # No prefix
    ("$", True),  # Variable prefix (for rewrite patterns)
])
def test_identifier_prefixes(prefix, valid):
    """Test that identifiers with various prefixes are handled correctly."""
    ident = f"{prefix}Freedom"
    expr = ident  # Bare atom
    if valid:
        result = parse_expression(expr)
        assert result is not None
        assert result.name == ident
    else:
        with pytest.raises(ParseError):
            parse_expression(expr)
