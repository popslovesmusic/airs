"""
Unit tests for SID parser in isolation.
Addresses Minor Issue #1.
"""
from __future__ import annotations

from sid_parser import parse_expression, tokenize, ParseError, OPERATORS


def test_tokenize_basic():
    """Test basic tokenization."""
    tokens = tokenize("P(Freedom)")
    assert len(tokens) == 4  # P, (, Freedom, )
    assert tokens[0].kind == "op"
    assert tokens[0].value == "P"
    assert tokens[1].kind == "lparen"
    assert tokens[2].kind == "ident"
    assert tokens[2].value == "Freedom"
    assert tokens[3].kind == "rparen"
    print("PASS: test_tokenize_basic")


def test_tokenize_operators():
    """Test all operator tokenization."""
    for op in OPERATORS:
        tokens = tokenize(f"{op}(x)")
        assert tokens[0].value == op
    print("PASS: test_tokenize_operators")


def test_tokenize_whitespace():
    """Test whitespace handling."""
    tokens = tokenize("  P  (  Freedom  )  ")
    # Whitespace tokens are filtered out
    assert all(t.kind != "ws" for t in tokens)
    assert len(tokens) == 4
    print("PASS: test_tokenize_whitespace")


def test_tokenize_line_column_tracking():
    """Test line/column tracking."""
    tokens = tokenize("P(\nFreedom\n)")
    assert tokens[0].line == 1
    assert tokens[0].column == 1
    # After P and ( and newline
    assert tokens[2].line == 2
    print("PASS: test_tokenize_line_column_tracking")


def test_tokenize_invalid_char():
    """Test error on invalid character."""
    try:
        tokenize("P(@)")
        assert False, "Should raise ParseError"
    except ParseError as e:
        assert "@" in str(e)
    print("PASS: test_tokenize_invalid_char")


def test_parse_atom():
    """Test parsing bare atom."""
    from sid_ast import Atom
    expr = parse_expression("Freedom")
    assert isinstance(expr, Atom)
    assert expr.name == "Freedom"
    print("PASS: test_parse_atom")


def test_parse_simple_op():
    """Test parsing simple operator."""
    from sid_ast import OpExpr
    expr = parse_expression("P(Freedom)")
    assert isinstance(expr, OpExpr)
    assert expr.op == "P"
    assert len(expr.args) == 1
    print("PASS: test_parse_simple_op")


def test_parse_nested():
    """Test parsing nested expression."""
    expr = parse_expression("O(P(Freedom))")
    from sid_ast import OpExpr
    assert isinstance(expr, OpExpr)
    assert expr.op == "O"
    assert len(expr.args) == 1
    assert expr.args[0].op == "P"
    print("PASS: test_parse_nested")


def test_parse_multiple_args():
    """Test parsing multiple arguments."""
    expr = parse_expression("C(P(A), P(B))")
    assert expr.op == "C"
    assert len(expr.args) == 2
    print("PASS: test_parse_multiple_args")


def test_parse_variable_syntax():
    """Test parsing variable with $ prefix."""
    expr = parse_expression("P($x)")
    assert expr.args[0].name == "$x"
    print("PASS: test_parse_variable_syntax")


def run_all_tests():
    """Run all parser unit tests."""
    print("Running parser unit tests...\n")

    test_tokenize_basic()
    test_tokenize_operators()
    test_tokenize_whitespace()
    test_tokenize_line_column_tracking()
    test_tokenize_invalid_char()
    test_parse_atom()
    test_parse_simple_op()
    test_parse_nested()
    test_parse_multiple_args()
    test_parse_variable_syntax()

    print("\nPASS: All parser unit tests passed!")


if __name__ == "__main__":
    run_all_tests()
